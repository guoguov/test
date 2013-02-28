#include <stdio.h>
#include <unistd.h>

#include "vgps.h"

void gps_message_handler(int fd)
{
	char buf[BUFSIZ];
	ssize_t n;
	
	while ((n = read(fd, buf, BUFSIZ)) > 0) {
		if (write(STDOUT_FILENO, buf, n) < 0) {
			perror("write");
			break;
		}
	}
	if (n < 0) 
		perror("read");
}