#include <time.h>
#include "loc_eng.h"
#include "utility.h"

static int loc_eng_init(GpsCallbacks *);
static int loc_eng_start(void);
static int loc_eng_stop(void);
static void loc_eng_cleanup(void);
static int loc_eng_set_position_mode(GpsPositionMode, GpsPositionRecurrence,
		uint32_t, uint32_t, uint32_t);
static const void* loc_eng_get_extension(const char *);
		
static const GpsInterface LocEngInterface = {
	.size = sizeof(GpsInterface),
	.init = loc_eng_init,
	.start = loc_eng_start,
	.stop = loc_eng_stop,
	.cleanup = loc_eng_cleanup,
	.set_position_mode = loc_eng_set_position_mode,
	.get_extension = loc_eng_get_extension,
};

static void loc_eng_nmea_event(const char *, int);
static void loc_eng_location_event(unsigned char, const void *); 

static struct gps_events loc_eng_events = {
	.nmea_event = loc_eng_nmea_event,
	.location_event = loc_eng_location_event,
};


static struct loc_eng_data_t loc_eng_data;

const GpsInterface* gps_get_hardware_interface(void)
{
	return &LocEngInterface;
}

static int loc_eng_init(GpsCallbacks *callbacks)
{
	loc_eng_data.location_cb = callbacks->location_cb;
	loc_eng_data.status_cb = callbacks->status_cb;
	loc_eng_data.sv_status_cb = callbacks->sv_status_cb;
	loc_eng_data.nmea_cb = callbacks->nmea_cb;
	
	loc_eng_data.device = get_gps_device();
	if (loc_eng_data.device == NULL) {
		LOGD("get_gps_device error");	
		return -1;
	}
	
	loc_eng_data.device->ops->init(&loc_eng_events);
	return 0;
}

static int loc_eng_start(void)
{
	struct ublox_gps_t *device = loc_eng_data.device;
	
	if (!device) {
		LOGD("device not initialized");
		return -1;
	}
	device->ops->start();
	return 0;
}

static int loc_eng_stop(void)
{
	struct ublox_gps_t *device = loc_eng_data.device;
	
	if (!device) {
		LOGD("device not initialized");
		return -1;
	}
	device->ops->stop();
	return 0;
}

static void loc_eng_cleanup(void)
{
	if (loc_eng_data.device) 
		loc_eng_data.device->ops->stop();	
}

static int loc_eng_set_position_mode(GpsPositionMode mode, 
		GpsPositionRecurrence recurrence,
		uint32_t min_interval, uint32_t preferred_accuraccy, uint32_t preferred_time)
{
	return 0;
}		

static const void* loc_eng_get_extension(const char * name)
{
	return NULL;
}

static void loc_eng_nmea_event(const char *nmea, int len)
{
	GpsUtcTime timestamp = (GpsUtcTime)utc() * 1000;
	loc_eng_data.nmea_cb(timestamp, nmea, len);
}

static GpsUtcTime __merge_date_utc(char date[], char utc[])
{
	struct tm tm;
	
	tm.tm_sec = (utc[4] - '0') * 10 + (utc[5] - '0');
	tm.tm_min = (utc[2] - '0') * 10 + (utc[3] - '0');
	tm.tm_hour = (utc[0] - '0') * 10 + (utc[1] - '0');
	tm.tm_mday = (date[0] - '0') * 10 + (date[1] - '0');
	tm.tm_mon = (date[2] - '0') * 10 + (date[3] - '0') - 1;
	tm.tm_year = (date[4] - '0') * 10 + (date[5] - '0') + 100;
	
	return (GpsUtcTime)mktime(&tm) * 1000;
}

static double __convert_lat_lon(double latlon)
{
	int deg;
	double min;
	
	deg = (int)latlon / 100;
	min = latlon - (deg * 100);
	return (min / 60 + deg);
}

static void loc_eng_location_event(unsigned char protocol, const void *data)
{
	if (protocol == PROT_NMEA0813) {
		struct nmea_message_t *p = (struct nmea_message_t *)data;
		
		if (p->mask & LAT_LON_SET) {
			GpsLocation location;
			
			location.size = sizeof(GpsLocation);
			location.flags = 0;
			
			location.latitude = __convert_lat_lon(p->latitude);
			location.longitude = __convert_lat_lon(p->longitude);
			location.flags |= GPS_LOCATION_HAS_LAT_LONG;
			
			if (p->mask & ALTITUDE_SET) {
				location.altitude = p->altitude;
				location.flags |= GPS_LOCATION_HAS_ALTITUDE;
			}
			
			if (p->mask & SPD_KNOT_SET) {
				location.speed = p->spd_knot * KNOT_2_MPS;
				location.flags |= GPS_LOCATION_HAS_SPEED;
			} else if (p->mask & SPD_KPH_SET) {
				location.speed = p->spd_kph * KPH_2_MPS;
				location.flags |= GPS_LOCATION_HAS_SPEED;
			}
			
			location.timestamp = __merge_date_utc(p->date, p->utc);
			loc_eng_data.location_cb(&location);
		}
		
		if ((p->mask & SV_INFO_SET) && (p->mask & SV_NUMBER_SET)) {
			GpsSvStatus sv_status;
			int i;
			
			sv_status.size = sizeof(GpsSvStatus);
			
			sv_status.num_svs = p->sv_in_view;
			for (i = 0; i < sv_status.num_svs; i++) {
				sv_status.sv_list[i].prn = (int)(p->sv_info[i].sv);
				sv_status.sv_list[i].snr = (float)(p->sv_info[i].CNo);
				sv_status.sv_list[i].elevation = (float)(p->sv_info[i].elevation);
				sv_status.sv_list[i].azimuth = (float)(p->sv_info[i].azimuth);
			}
			
			sv_status.ephemeris_mask = 0;
			sv_status.almanac_mask = 0;
			
			sv_status.used_in_fix_mask = 0;
			for (i = 0; i < GPS_MAX_SVS; i++) {
				if (p->sv_number[i] == 0)
					break;
				sv_status.used_in_fix_mask |= (1 << (p->sv_number[i] - 1));	
			}
			
			loc_eng_data.sv_status_cb(&sv_status);
		}
	}
}
