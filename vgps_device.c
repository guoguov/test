#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "vgps_device.h"

#define PATH		"NMEA.UBX"

static int start(int fd)
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

static int pipefds[2];
static pid_t pid = -1;

int open_vgps_port(void)
{
	if (pid > 0) 
		return pipefds[0];

	if (pipe(pipefds) < 0) {
		perror("pipe");
		return -1;
	}
	
	if ((pid = fork()) < 0) {
		perror("fork");
		return -1;
	} else if (pid == 0) {
		close(pipefds[0]);
		
		start(pipefds[1]);
		
		close(pipefds[1]);
		_exit(0);
	}
	
	close(pipefds[1]);
	return pipefds[0];
}

void close_vgps_port(void)
{
	if (pid > 0) {
		kill(pid, SIGKILL);
		close(pipefds[0]);
		waitpid(pid, NULL, 0);
		pid = -1;
	}
}
