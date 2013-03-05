#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utility.h"

// time

const char *timestamp(void)
{
	time_t tm;
	if (time(&tm) < 0) {
		perror("time");
		return NULL;
	}
	
	return ctime(&tm);
}

unsigned utc(void)
{
	time_t tm;
	if (time(&tm) < 0) {
		perror("time");
		return 0;
	}
	
	return (unsigned)tm;
}

// log


static int logfd = -1;

void systemlogger(int flag, const char *fmt, ...)
{
	va_list ap;
	int ret;
	char buf[BUFSIZ], temp[BUFSIZ];
	
	if (logfd < 0) {
		logfd = open(LOG_PATH, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR);
		if (logfd < 0) {
			perror("open");
			return;
		}
	}
	
	va_start(ap, fmt);
	ret = vsprintf(temp, fmt, ap);
	va_end(ap);
	if (ret < 0) {
		perror("vsprintf");
		return;
	}
	
	ret = snprintf(buf, BUFSIZ, "%s%s\n", 
			timestamp(), temp);
	if (ret < 0) {
		perror("snprintf");
		return;
	}		
	
	if (write(logfd, buf, strlen(buf)) < 0) {
		perror("write");
	}
	
	if (flag) {
		strcat(temp, "\n");
		write(STDOUT_FILENO, temp, strlen(temp));
	}	
}

