#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "loc_eng.h"

static void location_callback(GpsLocation *location)
{
	if (location->flags & GPS_LOCATION_HAS_LAT_LONG)
		printf("latitude: %f\tlongitude: %f\t", location->latitude, location->longitude);
	if (location->flags & GPS_LOCATION_HAS_ALTITUDE)
		printf("altitude: %f\t", location->altitude);
	if (location->flags & GPS_LOCATION_HAS_SPEED)
		printf("speed: %f\t", location->speed);
	printf("\n");	
}

static void status_callback(GpsStatus *status)
{
	switch (status->status) {
	case GPS_STATUS_NONE:
		printf("GPS status unknown\n");
		break;
	case GPS_STATUS_SESSION_BEGIN:
		printf("GPS has begun navigation\n");
		break;
	case GPS_STATUS_SESSION_END:
		printf("GPS has stopped navigation\n");
		break;
	case GPS_STATUS_ENGINE_ON:
		printf("GPS has powered on but is not navigation\n");
		break;
	case GPS_STATUS_ENGINE_OFF:
		printf("GPS has powered off\n");
		break;
	default:	
		printf("GPS status unknown\n");
	}
}

static void sv_status_callback(GpsSvStatus *sv_info)
{
	int i;
	
	for (i = 0; i < sv_info->num_svs; i++) {
		printf("G%d, %f %f %f\n", sv_info->sv_list[i].prn, 
				sv_info->sv_list[i].snr, 
				sv_info->sv_list[i].elevation,
				sv_info->sv_list[i].azimuth);
	}
	
	printf("sv used: ");
	for (i = 0; i < GPS_MAX_SVS; i++) {
		if (sv_info->used_in_fix_mask & (1 << i))
			printf("G%d ", i+1);
	}
	printf("\n");
}

static void nmea_callback(GpsUtcTime timestamp, const char *nmea, int length)
{
	printf("NMEA\n");
}

static GpsCallbacks callbacks = {
	.size = sizeof(GpsCallbacks),
	.location_cb = location_callback,
	.status_cb = status_callback,
	.sv_status_cb = sv_status_callback,
	.nmea_cb = nmea_callback,
};

int main(int argc, char *argv[])
{
	const GpsInterface *interface;
	
	interface = gps_get_hardware_interface();
	interface->init(&callbacks);
	interface->start();
	sleep(60);
	interface->stop();
	
	return 0;
}
