#include <stdio.h>
#include <unistd.h>

#include "vgps.h"
#include "driver_nmea0813.h"
#include "utility.h"

void gps_message_handler(int fd)
{
	char buf[BUFSIZ];
	ssize_t n;
	
	while ((n = read(fd, buf, BUFSIZ)) > 0) {
		if (write(STDOUT_FILENO, buf, n) < 0) {
			perror("write");
			break;
		}
		buf[n] = 0;
		if (parse_nmea(buf) < 0)
			LOGD("Error NMEA Message: %s", buf);
	}
	if (n < 0) 
		perror("read");
}