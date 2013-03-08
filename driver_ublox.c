#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "driver_ublox.h"
#include "driver_nmea0813.h"
#include "vgps_device.h"
#include "utility.h"

static int ublox_init(struct gps_events *);
static int ublox_start(void);
static int ublox_stop(void);

static struct gps_operations ublox_ops = {
	.init = ublox_init,
	.start = ublox_start,
	.stop = ublox_stop,
};

static struct ublox_gps_t ublox_gps;
static int GpsDeviceInitialized = 0;

struct ublox_gps_t* get_gps_device(void)
{
	if (!GpsDeviceInitialized) {
		ublox_gps.fd = -1;
		ublox_gps.protocol = PROT_NMEA0813;
		ublox_gps.data = NULL;
		ublox_gps.ops = &ublox_ops;
		ublox_gps.events = NULL;
		GpsDeviceInitialized = 1;
	}
	
	return &ublox_gps;
}

static int ublox_init(struct gps_events *events)
{
	ublox_gps.events = events;
	return 0;
}

static void gps_message_handler(int fd)
{
	char buf[BUFSIZ];
	ssize_t n;
	
	while ((n = read(fd, buf, BUFSIZ)) > 0) {
		if (write(STDOUT_FILENO, buf, n) < 0) {
			perror("write");
			break;
		}
		buf[n] = 0;
		ublox_gps.events->nmea_event(buf, n);
		if (parse_nmea(buf) < 0)
			LOGD("Error NMEA Message: %s", buf);	
	}
	if (n < 0) 
		perror("read");
}

static pid_t pid;

static int ublox_start(void)
{
	int fd;
	
	if (ublox_gps.fd >= 0)
		return 1;
	
	fd = open_vgps_port();
	if (fd < 0)
		return -1;
	ublox_gps.fd = fd;
	
	if ((pid = fork()) < 0) {
		perror("fork");
		close_vgps_port();
		return -1;
		
	} else if (pid == 0) {
		gps_message_handler(ublox_gps.fd);
		close(ublox_gps.fd);
		_exit(0);
	}
	
	return 0;	
}

static int ublox_stop(void)
{
	close_vgps_port();
	waitpid(pid, NULL, 0);
	return 0;
}

void deliver_nmea(const void *nmea)
{
	ublox_gps.events->location_event(PROT_NMEA0813, nmea);
}
