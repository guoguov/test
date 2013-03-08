#ifndef _GPS_H
#define _GPS_H

#include <stdint.h>
#include <sys/types.h>

/** Milliseconds since January 1, 1970 */
typedef int64_t GpsUtcTime;

/** Maximum number of SVs for gps_sv_status_callback(). */
#define GPS_MAX_SVS 32

/** Requested operational mode for GPS operation. */
typedef uint32_t GpsPositionMode;
/** Mode for running GPS standalone (no assistance). */
#define GPS_POSITION_MODE_STANDALONE    0
/** AGPS MS-Based mode. */
#define GPS_POSITION_MODE_MS_BASED      1
/** AGPS MS-Assisted mode. */
#define GPS_POSITION_MODE_MS_ASSISTED   2

/** Requested recurrence mode for GPS operation. */
typedef uint32_t GpsPositionRecurrence;
/** Receive GPS fixes on a recurring basis at a specified period. */
#define GPS_POSITION_RECURRENCE_PERIODIC    0
/** Request a single shot GPS fix. */
#define GPS_POSITION_RECURRENCE_SINGLE      1

/** GPS status event values. */
typedef uint16_t GpsStatusValue;
/** GPS status unknown. */
#define GPS_STATUS_NONE             0
/** GPS has begun navigating. */
#define GPS_STATUS_SESSION_BEGIN    1
/** GPS has stopped navigating. */
#define GPS_STATUS_SESSION_END      2
/** GPS has powered on but is not navigating. */
#define GPS_STATUS_ENGINE_ON        3
/** GPS is powered off. */
#define GPS_STATUS_ENGINE_OFF       4

/** Flags to indicate which values are valid in a GpsLocation. */
typedef uint16_t GpsLocationFlags;
/** GpsLocation has valid latitude and longitude. */
#define GPS_LOCATION_HAS_LAT_LONG   0x0001
/** GpsLocation has valid altitude. */
#define GPS_LOCATION_HAS_ALTITUDE   0x0002
/** GpsLocation has valid speed. */
#define GPS_LOCATION_HAS_SPEED      0x0004
/** GpsLocation has valid bearing. */
#define GPS_LOCATION_HAS_BEARING    0x0008
/** GpsLocation has valid accuracy. */
#define GPS_LOCATION_HAS_ACCURACY   0x0010

/** Flags for the gps_set_capabilities callback. */

/** GPS HAL schedules fixes for GPS_POSITION_RECURRENCE_PERIODIC mode.
    If this is not set, then the framework will use 1000ms for min_interval
    and will start and call start() and stop() to schedule the GPS.
 */
#define GPS_CAPABILITY_SCHEDULING       0x0000001
/** GPS supports MS-Based AGPS mode */
#define GPS_CAPABILITY_MSB              0x0000002
/** GPS supports MS-Assisted AGPS mode */
#define GPS_CAPABILITY_MSA              0x0000004
/** GPS supports single-shot fixes */
#define GPS_CAPABILITY_SINGLE_SHOT      0x0000008
/** GPS supports on demand time injection */
#define GPS_CAPABILITY_ON_DEMAND_TIME   0x0000010

/** Represents a location. */
typedef struct {
    /** set to sizeof(GpsLocation) */
    size_t          size;
    /** Contains GpsLocationFlags bits. */
    uint16_t        flags;
    /** Represents latitude in degrees. */
    double          latitude;
    /** Represents longitude in degrees. */
    double          longitude;
    /** Represents altitude in meters above the WGS 84 reference
     * ellipsoid. */
    double          altitude;
    /** Represents speed in meters per second. */
    float           speed;
    /** Represents heading in degrees. */
    float           bearing;
    /** Represents expected accuracy in meters. */
    float           accuracy;
    /** Timestamp for the location fix. */
    GpsUtcTime      timestamp;
} GpsLocation;

/** Represents the status. */
typedef struct {
    /** set to sizeof(GpsStatus) */
    size_t          size;
    GpsStatusValue status;
} GpsStatus;

/** Represents SV information. */
typedef struct {
    /** set to sizeof(GpsSvInfo) */
    size_t          size;
    /** Pseudo-random number for the SV. */
    int     prn;
    /** Signal to noise ratio. */
    float   snr;
    /** Elevation of SV in degrees. */
    float   elevation;
    /** Azimuth of SV in degrees. */
    float   azimuth;
} GpsSvInfo;

/** Represents SV status. */
typedef struct {
    /** set to sizeof(GpsSvStatus) */
    size_t          size;

    /** Number of SVs currently visible. */
    int         num_svs;

    /** Contains an array of SV information. */
    GpsSvInfo   sv_list[GPS_MAX_SVS];

    /** Represents a bit mask indicating which SVs
     * have ephemeris data.
     */
    uint32_t    ephemeris_mask;

    /** Represents a bit mask indicating which SVs
     * have almanac data.
     */
    uint32_t    almanac_mask;

    /**
     * Represents a bit mask indicating which SVs
     * were used for computing the most recent position fix.
     */
    uint32_t    used_in_fix_mask;
} GpsSvStatus;

/** Callback with location information. */
typedef void (* gps_location_callback)(GpsLocation* location);

/** Callback with status information. */
typedef void (* gps_status_callback)(GpsStatus* status);

/** Callback with SV status information. */
typedef void (* gps_sv_status_callback)(GpsSvStatus* sv_info);

/** Callback for reporting NMEA sentences. */
typedef void (* gps_nmea_callback)(GpsUtcTime timestamp, const char* nmea, int length);

/** Callback to inform framework of the GPS engine's capabilities. */
typedef void (* gps_set_capabilities)(uint32_t capabilities);

/** Callback for requesting NTP time */
typedef void (* gps_request_utc_time)();

/** GPS callback structure. */
typedef struct {
    /** set to sizeof(GpsCallbacks) */
    size_t      size;
    gps_location_callback location_cb;
    gps_status_callback status_cb;
    gps_sv_status_callback sv_status_cb;
    gps_nmea_callback nmea_cb;
    gps_set_capabilities set_capabilities_cb;
    gps_request_utc_time request_utc_time_cb;
} GpsCallbacks;

/** Represents the standard GPS interface. */
typedef struct {
    /** set to sizeof(GpsInterface) */
    size_t          size;
    /**
     * Opens the interface and provides the callback routines
     * to the implemenation of this interface.
     */
    int   (*init)( GpsCallbacks* callbacks );

    /** Starts navigating. */
    int   (*start)( void );

    /** Stops navigating. */
    int   (*stop)( void );

    /** Closes the interface. */
    void  (*cleanup)( void );

    /** Injects the current time. */
    // int   (*inject_time)(GpsUtcTime time, int64_t timeReference,
                         // int uncertainty);

    /** Injects current location from another location provider
     *  (typically cell ID).
     *  latitude and longitude are measured in degrees
     *  expected accuracy is measured in meters
     */
    // int  (*inject_location)(double latitude, double longitude, float accuracy);

    /**
     * Specifies that the next call to start will not use the
     * information defined in the flags. GPS_DELETE_ALL is passed for
     * a cold start.
     */
    // void  (*delete_aiding_data)(GpsAidingData flags);

    /**
     * min_interval represents the time between fixes in milliseconds.
     * preferred_accuracy represents the requested fix accuracy in meters.
     * preferred_time represents the requested time to first fix in milliseconds.
     */
    int   (*set_position_mode)(GpsPositionMode mode, GpsPositionRecurrence recurrence,
            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time);

    /** Get a pointer to extension information. */
    const void* (*get_extension)(const char* name);
} GpsInterface;

#endif