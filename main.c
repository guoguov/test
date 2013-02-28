#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "vgps.h"
#include "vgps_device.h"

int main(int argc, char *argv[])
{
	int pipefds[2];
	pid_t pid;
	
	if (pipe(pipefds) < 0) {
		perror("pipe");
		exit(1);
	}
	
	pid = fork();
	if (pid == 0) {	// write pipe
		close(pipefds[0]);
		
		open_vgps_port(pipefds[1]);
		
		close(pipefds[1]);
		_exit(0);
	
	} else {		// read pipe
		close(pipefds[1]);
		
		gps_message_handler(pipefds[0]);
		
		close(pipefds[0]);
		wait(NULL);
		exit(0);
	}
	
	return 0;
}
