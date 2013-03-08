#ifndef _LOC_ENG_H
#define _LOC_ENG_H

#include "gps.h"
#include "driver_ublox.h"

struct loc_eng_data_t {
	gps_location_callback location_cb;
	gps_status_callback status_cb;
	gps_sv_status_callback sv_status_cb;
	gps_nmea_callback nmea_cb;
	
	struct ublox_gps_t *device;
};

const GpsInterface* gps_get_hardware_interface(void);

#endif