#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver_nmea0813.h"
#include "utility.h"

#define FIELD_SIZ		32
static char Fields[32][FIELD_SIZ];

static int extract_nmea_field(const char *sentence, char *field[])
{
	int i, j;
	const char *p = sentence;
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	
	i = 0;
	for (;;) {
		for (j = 0; *p != ',' && *p != '*' && *p != '\0'; p++) {
			fieldp[i][j++] = *p;
			if (*p == '\n')
				break;
		}
		fieldp[i++][j] = '\0';
		if (*p == '\0' || *p == '\n')
			break;
		p++;	
	}
	*fieldp[i] = 0;
	
	return i;
}

static unsigned char nmea_checksum(const char *sentence)
{
	unsigned char sum = 0;
	char c; 
	const char *p = sentence;
	
	if (*p == '$')
		p++;
	while (((c = *p) != '*') && (c != '\0')) {
		sum ^= c;
		p++;
    } 	
	
	return sum;
}

static void report_nmea(const struct nmea_message_t *session)
{
	int i;
	char temp[128], s[8], *p;

	if (session->mask & LAT_LON_SET) {
		LOGD("latitude: %f(%c), longitude: %f(%c)", 
				session->latitude, session->NS_ind, 
				session->longitude, session->EW_ind);
	}
	if (session->mask & ALTITUDE_SET) {
		LOGD("altitude: %f", session->altitude);
	}
	if (session->mask & SPD_KNOT_SET) {
		LOGD("speed: %f knots", session->spd_knot);
	}
	if (session->mask & SPD_KPH_SET) {
		LOGD("speed: %f km/h", session->spd_kph);
	}
	if (session->mask & COG_SET) {
		LOGD("cog: %f", session->cog);
	}
	if (session->mask & DOP_SET) {
		LOGD("PDOP: %f, HDOP: %f, VDOP: %f", 
				session->PDOP, session->HDOP, session->VDOP);
	}
	if (session->mask & SV_INFO_SET) {
		LOGD("%d satillites in view", session->sv_in_view);
		for (i = 0; i < session->sv_in_view && i < GPS_MAX_SVS; i++) {
			LOGD("G%d, %d(elevation) %d(azimuth) %d(C/No)",
					session->sv_info[i].sv, session->sv_info[i].elevation,
					session->sv_info[i].azimuth, session->sv_info[i].CNo);
		}
	}
	if (session->mask & SV_NUMBER_SET) {
		sprintf(temp, "%d satillites used, ", session->sv_used);
		for (i = 0; i < GPS_MAX_SVS; i++) {
			if (session->sv_number[i] == 0)
				break;
			sprintf(s, "G%d ", session->sv_number[i]);
			strcat(temp, s);	
		}
		LOGD("%s", temp);
	}
	if (session->mask & GPS_STATUS_SET) {
		if (session->status == STATUS_VALID)
			LOGD("STATUS_VALID");
		else
			LOGD("STATUS_INVALID");
	}
	if (session->mask & GPS_QUALITY_SET) {
		if (session->quality == QUALITY_GPS_SPS)
			LOGD("QUALITY_GPS_SPS");
		else if (session->quality == QUALITY_DGPS_SPS)
			LOGD("QUALITY_DGPS_SPS");
		else if (session->quality == QUALITY_ESTIMATE_DEAD_ROCKONING)
			LOGD("QUALITY_ESTIMATE_DEAD_ROCKONING");
		else
			LOGD("QUALITY_INVALID");
	}
	if (session->mask & NAV_MODE_SET) {
		if (session->nav_mode == NAVMOD_2D_FIX)
			LOGD("NAVMOD_2D_FIX");
		else if (session->nav_mode == NAVMOD_3D_FIX)
			LOGD("NAVMOD_3D_FIX");
		else
			LOGD("NAVMOD_NO_FIX");
	}
	if (session->mask & GPS_MODE_SET) {
		if (session->mode == MODE_AUTO_GNSS) 
			LOGD("MODE_AUTO_GNSS");
		else if (session->mode == MODE_DIFF_GNSS)
			LOGD("MODE_DIFF_GNSS");
		else if (session->mode == MODE_ESTIMATE_DEAD_ROCKONING)
			LOGD("MODE_ESTIMATE_DEAD_ROCKONING");
		else
			LOGD("MODE_NO_FIX");
	}
	if (session->mask & UTC_SET) {
		p = (char *)session->utc;
		LOGD("%c%c:%c%c:%c%c%c%c%c%c", 
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9]);
	}
	if (session->mask & DATE_SET) {
		p = (char *)session->date;
		LOGD("%c%c-%c%c-%c%c", 
				p[0], p[1], p[2], p[3], p[4], p[5]);
	}
}

unsigned processGPGGA(int count, const char *field[], struct nmea_message_t *session)
{
	unsigned mask = 0;
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	
	switch (*fieldp[6]) {
	case '0':
		session->quality = QUALITY_INVALID;
		break;
	case '1':
		session->quality = QUALITY_GPS_SPS;
		break;
	case '2':
		session->quality = QUALITY_DGPS_SPS;
		break;
	case '6':
		session->quality = QUALITY_ESTIMATE_DEAD_ROCKONING;
		break;
	default:
		session->quality = QUALITY_INVALID;
	}
	if (session->quality == QUALITY_INVALID)
		return 0;
	mask |= GPS_QUALITY_SET;	
	
	memcpy(session->utc, fieldp[1], 10);
	mask |= UTC_SET;
	
	session->latitude = atof(fieldp[2]);
	session->NS_ind = *fieldp[3];
	session->longitude = atof(fieldp[4]);
	session->EW_ind = *fieldp[5];
	mask |= LAT_LON_SET;
	session->sv_used = atoi(fieldp[7]);
	
	if (*fieldp[9] != 0) {
		session->altitude = atof(fieldp[9]);
		mask |= ALTITUDE_SET;
	}
	
	session->mask |= mask;
	return mask;
}

unsigned processGPGLL(int count, const char *field[], struct nmea_message_t *session)
{
	unsigned mask = 0;
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	
	if (*fieldp[6] != 'A') 
		return 0;
	session->status = STATUS_VALID;
	mask |= GPS_STATUS_SET;	
	
	session->latitude = atof(fieldp[1]);
	session->NS_ind = *fieldp[2];
	session->longitude = atof(fieldp[3]);
	session->EW_ind = *fieldp[4];
	mask |= LAT_LON_SET;
	
	memcpy(session->utc, fieldp[5], 10);
	mask |= UTC_SET;
	
	session->mask |= mask;
	return mask;
}

unsigned processGPRMC(int count, const char *field[], struct nmea_message_t *session)
{
	unsigned mask = 0;
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	
	if (*fieldp[2] != 'A') 
		return 0;
	session->status = STATUS_VALID;
	mask |= GPS_STATUS_SET;
	
	memcpy(session->utc, fieldp[1], 10);
	mask |= UTC_SET;
	
	session->latitude = atof(fieldp[3]);
	session->NS_ind = *fieldp[4];
	session->longitude = atof(fieldp[5]);
	session->EW_ind = *fieldp[6];
	mask |= LAT_LON_SET;
	
	session->spd_knot = atof(fieldp[7]);
	mask |= SPD_KNOT_SET;
	
	session->cog = atof(fieldp[8]);
	mask |= COG_SET;
	
	memcpy(session->date, fieldp[9], 6);
	mask |= DATE_SET;
	
	switch (*fieldp[12]) {
	case 'N':
		session->mode = MODE_NO_FIX;
		break;
	case 'E':
		session->mode = MODE_ESTIMATE_DEAD_ROCKONING;
		break;
	case 'A':
		session->mode = MODE_AUTO_GNSS;
		break;
	case 'D':
		session->mode = MODE_DIFF_GNSS;
		break;
	default:
		session->mode = MODE_NO_FIX;
	}
	mask |= GPS_MODE_SET;
	
	session->mask |= mask;
	return mask;
}

unsigned processGPVTG(int count, const char *field[], struct nmea_message_t *session)
{
	unsigned mask = 0;
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	
	switch (*fieldp[9]) {
	case 'N':
		session->mode = MODE_NO_FIX;
		break;
	case 'E':
		session->mode = MODE_ESTIMATE_DEAD_ROCKONING;
		break;
	case 'A':
		session->mode = MODE_AUTO_GNSS;
		break;
	case 'D':
		session->mode = MODE_DIFF_GNSS;
		break;
	default:
		session->mode = MODE_NO_FIX;
	}
	if (session->mode == MODE_NO_FIX)
		return 0;
	mask |= GPS_MODE_SET;
	
	session->spd_knot = atof(fieldp[5]);
	mask |= SPD_KNOT_SET;
	
	session->spd_kph = atof(fieldp[7]);
	mask |= SPD_KPH_SET;
	
	session->mask |= mask;
	return mask;
}

unsigned processGPGSA(int count, const char *field[], struct nmea_message_t *session)
{
	unsigned mask = 0;
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	int i;
	
	memset(session->sv_number, 0, GPS_MAX_SVS);
	for (i = 0; i < 12; i++) {
		if (*fieldp[i+3]) 
			session->sv_number[i] = atoi(fieldp[i+3]);
		else
			break;
	}
	mask |= SV_NUMBER_SET;
	
	if (*fieldp[15] && *fieldp[16] && *fieldp[17])	{
		session->PDOP = atof(fieldp[15]);
		session->HDOP = atof(fieldp[16]);
		session->VDOP = atof(fieldp[17]);
		mask |= DOP_SET;
	} 
	
	session->mask |= mask;
	return mask;
}

unsigned processGPGSV(int count, const char *field[], struct nmea_message_t *session)
{
	unsigned mask = 0;
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	static int sv_info_count = 0;
	static int NoMsg = 0;
	static int MsgNo = 0;
	int temp, i;
	
	session->sv_in_view = atoi(fieldp[3]);
	
	temp = atoi(fieldp[1]);
	if (temp == 0 || (NoMsg != 0 && temp != NoMsg))
		return 0;
	if (NoMsg == 0)
		NoMsg = temp;
		
	temp = atoi(fieldp[2]);
	if (temp != (MsgNo + 1)) {
		NoMsg = 0;
		sv_info_count = 0;
		return 0;
	}
	MsgNo++;
	
	for (i = 0; sv_info_count < session->sv_in_view; i += 4) {
		if (*fieldp[i+4]) {
			session->sv_info[sv_info_count].sv = (unsigned char)atoi(fieldp[i+4]);
			if (*fieldp[i+5])
				session->sv_info[sv_info_count].elevation = (unsigned char)atoi(fieldp[i+5]);
			if (*fieldp[i+6])	
				session->sv_info[sv_info_count].azimuth = (unsigned short)atoi(fieldp[i+6]);
			if (*fieldp[i+7])	
				session->sv_info[sv_info_count].CNo = (unsigned char)atoi(fieldp[i+7]);
			sv_info_count++;
		} else
			break;
	}
	
	if (MsgNo == NoMsg) {
		if (sv_info_count == session->sv_in_view)
			mask |= SV_INFO_SET;
		sv_info_count = 0;
		NoMsg = 0;
		MsgNo = 0;
	}
	
	session->mask |= mask;
	return mask;
}

unsigned processGPTXT(int count, const char *field[], struct nmea_message_t *session)
{
	char (*fieldp)[FIELD_SIZ] = (char (*)[FIELD_SIZ])field;
	const char MessageLevel[4][8] = { "ERROR", "WARNING", "NOTICE", "USER" };
	int level;
	
	level = atoi(fieldp[3]);
	if (level > 3 || level < 0)
		level = 3;
	LOGD("%s: %s", MessageLevel[level], fieldp[4]);
	return 0;
}


static struct nmea_message_t session = {
	.mask = 0,
};

int parse_nmea(const char *sentence)
{
	int count;
	char checksum[5];
	const char** fieldp = (const char **)Fields;
	
	count = extract_nmea_field(sentence, (char **)Fields);
	if (count <= 0) {
		LOGD("extract_nmea_field error");
		return -1;
	}	
	if (*Fields[0] != '$') {
		LOGD("Head error: %c", *Fields[0]);
		return -1;
	}	
	snprintf(checksum, 5, "%02X\r\n", nmea_checksum(sentence));
	if (strncmp(Fields[count-1], checksum, 4) != 0) {
		LOGD("checksum error, %s, %s", checksum, Fields[count-1]);
		return -1;
	}	
		
	if (strncmp(Fields[0], "$GPGGA", 6) == 0)
		processGPGGA(count, fieldp, &session);
	else if (strncmp(Fields[0], "$GPGLL", 6) == 0)
		processGPGLL(count, fieldp, &session);
	else if (strncmp(Fields[0], "$GPRMC", 6) == 0)
		processGPRMC(count, fieldp, &session);
	else if (strncmp(Fields[0], "$GPVTG", 6) == 0)
		processGPVTG(count, fieldp, &session);
	else if (strncmp(Fields[0], "$GPGSA", 6) == 0)
		processGPGSA(count, fieldp, &session);
	else if (strncmp(Fields[0], "$GPGSV", 6) == 0)
		processGPGSV(count, fieldp, &session);
	else if (strncmp(Fields[0], "$GPTXT", 6) == 0)
		processGPTXT(count, fieldp, &session);
	else if (strncmp(Fields[0], "$GPDTM", 6) == 0)
		LOGI("GPDTM");
	else if (strncmp(Fields[0], "$GPGBS", 6) == 0)
		LOGI("GPGBS");
	else if (strncmp(Fields[0], "$GPGPQ", 6) == 0)
		LOGI("GPGPQ");
	else if (strncmp(Fields[0], "$GPGRS", 6) == 0)
		LOGI("GPGRS");
	else if (strncmp(Fields[0], "$GPZDA", 6) == 0)
		LOGI("GPZDA");
	else {
		LOGD("Unkown NMEA Message");
		return -1;
	}
	
	if ((session.mask & LAT_LON_SET) || (session.mask & SV_INFO_SET)) {
		report_nmea(&session);
		session.mask = 0;
	}	
		
	return 0;	
}