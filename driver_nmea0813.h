#ifndef _DRIVER_NMEA0813_H
#define _DRIVER_NMEA0813_H

struct nmea_message_t {
	unsigned mask;
	#define LAT_LON_SET				0x00000001
	#define ALTITUDE_SET			0x00000002
	#define SPD_KNOT_SET			0x00000004
	#define SPD_KPH_SET				0x00000008
	#define COG_SET					0x00000010
	#define DOP_SET					0x00000020
	#define SV_INFO_SET				0x00000040
	#define SV_NUMBER_SET			0x00000080
	#define GPS_STATUS_SET			0x00000100
	#define	GPS_QUALITY_SET			0x00000200
	#define NAV_MODE_SET			0x00000400
	#define GPS_MODE_SET			0x00000800
	#define UTC_SET					0x00001000
	#define	DATE_SET				0x00002000
	#define ALL_IS_SET				0x00003fff

	double latitude;
	double longitude;
	double altitude;
	char NS_ind;
	char EW_ind;
	#define NORTHERN_HEMISPHERE		78
	#define SOUTHERN_HEMISPHERE		83
	#define EASTERN_HEMISPHERE		69
	#define WESTERN_HEMISPHERE		87
	
	double spd_knot;
	double spd_kph;
	#define KNOT_2_KPH				1.852
	#define KNOT_2_MPS				0.514444
	#define KPH_2_MPS				0.277778
	double cog;
	
	double PDOP;
	double HDOP;
	double VDOP;
	
	#define GPS_MAX_SVS 			32
	unsigned char sv_in_view;
	struct {
		unsigned char sv;
		unsigned char elevation;
		unsigned short azimuth;
		unsigned char CNo;
	} sv_info[GPS_MAX_SVS];
	unsigned char sv_used;
	unsigned char sv_number[GPS_MAX_SVS];
	
	unsigned char status;
	#define STATUS_VALID			1
	#define STATUS_INVALID			0
	unsigned char quality;
	#define QUALITY_INVALID			0
	#define QUALITY_GPS_SPS			1
	#define QUALITY_DGPS_SPS		2
	#define QUALITY_ESTIMATE_DEAD_ROCKONING		6
	unsigned char nav_mode;
	#define NAVMOD_NO_FIX			1
	#define NAVMOD_2D_FIX			2
	#define NAVMOD_3D_FIX			3
	unsigned char mode;
	#define MODE_NO_FIX				0
	#define	MODE_AUTO_GNSS			1
	#define MODE_DIFF_GNSS			2
	#define MODE_ESTIMATE_DEAD_ROCKONING	3
	
	char utc[10];
	char date[6];
	unsigned tiemstamp;
};

int parse_nmea(const char *sentence);

#endif