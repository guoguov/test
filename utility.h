#ifndef _UTILITY_H
#define _UTILITY_H

const char *timestamp(void);
unsigned utc(void);

#define LOG_PATH		"sys.log"

void systemlogger(int flag, const char *fmt, ...);
#define LOGD(fmt, ...)	systemlogger(1, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...)	systemlogger(0, fmt, ##__VA_ARGS__)

#endif