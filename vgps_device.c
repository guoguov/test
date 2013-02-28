#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "vgps_device.h"

#define PATH		"NMEA.UBX"

int open_vgps_port(int fd)
{
	FILE *fp;
	char buf[BUFSIZ];
	
	fp = fopen(PATH, "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}
	
	while (fgets(buf, BUFSIZ, fp)) {			
		if (write(fd, buf, strlen(buf)) < 0) {
			perror("write");
			break;
		}	
		sleep(1);
	}
	
	fclose(fp);
	return 0;
}
