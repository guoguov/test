#ifndef _DRIVER_UBLOX_H
#define _DRIVER_UBLOX_H

#include "driver_nmea0813.h"

struct gps_events {
	void (*nmea_event)(const char *, int);
	void (*location_event)(unsigned char, const void *);
};

struct gps_operations {
	int (*init)(struct gps_events *);
	int (*start)(void);
	int (*stop)(void);
};

struct ublox_gps_t {
	int fd;
	unsigned char protocol;
	#define PROT_UBX			0
	#define PROT_NMEA0813		1
	void *data;
	struct gps_operations *ops;
	struct gps_events *events;
};

struct ublox_gps_t* get_gps_device(void);
void deliver_nmea(const void *nmea);

#endif