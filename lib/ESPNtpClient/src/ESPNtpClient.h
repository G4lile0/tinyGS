/**
  * @file ESPNtpClient.h
  * @version 0.2.5
  * @date 16/03/2021
  * @author German Martin
  * @brief Library to get system sync from a NTP server with microseconds accuracy in ESP8266 and ESP32
  */

#ifndef _EspNtpClient_h
#define _EspNtpClient_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <functional>
using namespace std;
using namespace placeholders;

extern "C" {
#include "lwip/init.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "sys/time.h"
#ifdef ESP32
#include "include/time.h"
#else
#include "time.h"
#endif
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
}

#ifdef ESP32
#include "TZdef.h"
#else
#include "TZ.h"
#endif

constexpr auto DEFAULT_NTP_SERVER = "pool.ntp.org"; ///< @brief Default international NTP server. I recommend you to select a closer server to get better accuracy
constexpr auto DEFAULT_NTP_PORT = 123; ///< @brief Default local udp port. Select a different one if neccesary (usually not needed)
constexpr auto DEFAULT_NTP_INTERVAL = 1800; ///< @brief Default sync interval 30 minutes
constexpr auto DEFAULT_NTP_SHORTINTERVAL = 15; ///< @brief Sync interval when sync has not been achieved. 15 seconds
constexpr auto DEFAULT_NTP_TIMEOUT = 5000; ///< @brief Default NTP timeout ms
//constexpr auto FAST_NTP_SYNCNTERVAL = DEFAULT_NTP_TIMEOUT * 1.1; ///< @brief Sync interval when sync has not reached required accuracy in ms
constexpr auto MIN_NTP_TIMEOUT = 250; ///< @brief Minumum admisible ntp timeout in ms
constexpr auto MIN_NTP_INTERVAL = 10; ///< @brief Minumum NTP request interval in seconds
constexpr auto DEFAULT_MIN_SYNC_ACCURACY_US = 5000; ///< @brief Minimum sync accuracy in us
constexpr auto DEFAULT_MAX_RESYNC_RETRY = 3; ///< @brief Maximum number of sync retrials if offset is above accuravy
constexpr auto DEAULT_NUM_TIMEOUTS = 3; ///< @brief After this number of timeouts there is no more continiuos
#ifdef ESP8266
constexpr auto ESP8266_LOOP_TASK_INTERVAL = 500; ///< @brief Loop task period on ESP8266
constexpr auto ESP8266_RECEIVER_TASK_INTERVAL = 100; ///< @brief Receiver task period on ESP8266
#endif // ESP8266
constexpr auto DEFAULT_TIME_SYNC_THRESHOLD = 2500; ///< @brief If calculated offset is less than this in us clock will not be corrected
constexpr auto DEFAULT_NUM_OFFSET_AVE_ROUNDS = 1; ///< @brief Number of NTP request and response rounds to calculate offset average
constexpr auto MAX_OFFSET_AVERAGE_ROUNDS = 5; ///< @brief Maximum number of NTP request for offset average calculation

constexpr auto TZNAME_LENGTH = 60; ///< @brief Max TZ name description length
constexpr auto SERVER_NAME_LENGTH = 40; ///< @brief Max server name (FQDN) length
constexpr auto NTP_PACKET_SIZE = 48; ///< @brief NTP time is in the first 48 bytes of message

/* Useful Constants */
#ifndef SECS_PER_MIN
constexpr auto SECS_PER_MIN = ((time_t)(60UL));
#endif
#ifndef SECS_PER_HOUR
constexpr auto SECS_PER_HOUR = ((time_t)(3600UL));
#endif
#ifndef SECS_PER_DAY
constexpr auto SECS_PER_DAY = ((time_t)(SECS_PER_HOUR * 24UL));
#endif
#ifndef DAYS_PER_WEEK
constexpr auto DAYS_PER_WEEK = ((time_t)(7UL));
#endif
#ifndef SECS_PER_WEEK
constexpr auto SECS_PER_WEEK = ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK));
#endif
#ifndef SECS_PER_YEAR
constexpr auto SECS_PER_YEAR = ((time_t)(SECS_PER_DAY * 365UL));
#endif
#ifndef SECS_YR_2000
constexpr auto SECS_YR_2000 = ((time_t)(946684800UL)); ///< @brief The time at the start of y2k
#endif

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <Ticker.h>

#include "NTPEventTypes.h"

  /**
    * @brief NTP client status code
    */
typedef enum NTPStatus {
    syncd = 0, // Time synchronized correctly
    unsyncd = -1, // Time may not be valid
    partialSync = 1 // NPT is synchronised but precission is below threshold
} NTPStatus_t; // Only for internal library use

  /**
    * @brief Flags in NTP packet
    */
typedef struct {
    /**
      * @brief 2-bit integer warning of an impending leap
      * second to be inserted or deleted in the last minute of the current
      * month with values defined in this table
      * 
      * | Value | Meaning |
      * |:----------:|-------------|
      * | 0     | no warning                             |
      * | 1     | last minute of the day has 61 seconds  |
      * | 2     | last minute of the day has 59 seconds  |
      * | 3     | unknown (clock unsynchronized)         |
      */
    int li;
    int vers; ///< @brief 3-bit integer representing the NTP version number, currently 4
    /**
      * @brief 3-bit integer representing the mode, with values defined in this table
      * 
      * | Value | Meaning                  |
      * |:-----:|--------------------------|
      * | 0     | reserved                 |
      * | 1     | symmetric active         |
      * | 2     | symmetric passive        |
      * | 3     | client                   |
      * | 4     | server                   |
      * | 5     | broadcast                |
      * | 6     | NTP control message      |
      * | 7     | reserved for private use |
      */
    int mode;
} NTPFlags_t;

  /**
    * @brief NTP packet structure
    */
typedef struct {
    bool valid = false; ///< @brief true if following data is valid
    NTPFlags_t flags; ///< @brief NTP packet flags as NTPFlags_t
    
    /**
      * @brief 8-bit integer representing the stratum
      * 
      * | Value  | Meaning                                             |
      * |:------:|-----------------------------------------------------|
      * | 0      | unspecified or invalid                              |
      * | 1      | primary server (e.g., equipped with a GPS receiver) |
      * | 2-15   | secondary server (via NTP)                          |
      * | 16     | unsynchronized                                      |
      * | 17-255 | reserved                                            |
      */
    uint8_t peerStratum;
    
     /**
      * @brief Maximum interval between successive messages, in seconds.
      *
      * Calculated from 8-bit signed integer representing log2 value. Suggested default limits for 
      * minimum and maximum poll intervals are 6 and 10, what represent 64 to 1024 seconds, respectively
      */
    uint32_t pollingInterval;
    
    /**
      * @brief 8-bit signed integer representing the precision of the
      * system clock, in log2 seconds
      * 
      * For instance, a value of -18 corresponds to a precision of about one microsecond.
      * The precision can be determined when the service first starts up as the minimum
      * time of several iterations to read the system clock
      */
    float clockPrecission;
       
    float rootDelay; ///< @brief Total round-trip delay to the reference clock
    
    float dispersion; ///< @brief Total dispersion to the reference clock
    
     /**
      * @brief 32-bit code identifying the particular server or reference clock
      * 
      * The interpretation depends on the value in the stratum field.
      * For packet stratum 0 (unspecified or invalid), this is a four-character ASCII [RFC1345] string,
      * called the "kiss code", used for debugging and monitoring purposes. For stratum 1 (reference
      * clock), this is a four-octet, left-justified, zero-padded ASCII string assigned to the
      * reference clock. 
      * 
      * The authoritative list of Reference Identifiers is maintained by IANA; however, any string
      * beginning with the ASCII character "X" is reserved for unregistered experimentation and 
      * development. Next identifiers have been used as ASCII identifiers:
      * 
      * | ID   | Clock Source                                             |
      * |:----:|----------------------------------------------------------|
      * | GOES | Geosynchronous Orbit Environment Satellite               |
      * | GPS  | Global Position System                                   |
      * | GAL  | Galileo Positioning System                               |
      * | PPS  | Generic pulse-per-second                                 |
      * | IRIG | Inter-Range Instrumentation Group                        |
      * | WWVB | LF Radio WWVB Ft. Collins, CO 60 kHz                     |
      * | DCF  | LF Radio DCF77 Mainflingen, DE 77.5 kHz                  |
      * | HBG  | LF Radio HBG Prangins, HB 75 kHz                         |
      * | MSF  | LF Radio MSF Anthorn, UK 60 kHz                          |
      * | JJY  | LF Radio JJY Fukushima, JP 40 kHz, Saga, JP 60 kHz       |
      * | LORC | MF Radio LORAN C station, 100 kHz                        |
      * | TDF  | MF Radio Allouis, FR 162 kHz                             |
      * | CHU  | HF Radio CHU Ottawa, Ontario                             |
      * | WWV  | HF Radio WWV Ft. Collins, CO                             |
      * | WWVH | HF Radio WWVH Kauai, HI                                  |
      * | NIST | NIST telephone modem                                     |
      * | ACTS | NIST telephone modem                                     |
      * | USNO | USNO telephone modem                                     |
      * | PTB  | European telephone modem                                 |
      * 
      * Above stratum 1 (secondary servers and clients): this is the reference identifier of
      * the server and can be used to detect timing loops. If using the IPv4 address family,
      * the identifier is the four-octet IPv4 address. If using the IPv6 address family, it is the
      * first four octets of the MD5 hash of the IPv6 address. Note that, when using the IPv6 address
      * family on an NTPv4 server with a NTPv3 client, the Reference Identifier field appears to be a 
      * random value and a timing loop might not be detected
      */
    uint8_t refID[4];
    
    timeval reference; ///< @brief Time when the system clock was last set or corrected
    timeval origin; ///< @brief Time at the client when the request departed for the server
    timeval receive; ///< @brief Time at the server when the request arrived from the client
    timeval transmit; ///< @brief Time at the server when the response left for the client
    timeval destination; ///< Time at the client when the reply arrived from the server, in NTP timestamp format
} NTPPacket_t;

typedef std::function<void (NTPEvent_t)> onSyncEvent_t; ///< @brief Event notifier callback

static char strBuffer[35]; ///< @brief Temporary buffer for time and date strings

/**
  * @brief NTPClient class
  */
class NTPClient {
protected:
    udp_pcb* udp;                   ///< @brief UDP connection object
    timeval lastSyncd;              ///< @brief Stored time of last successful sync
    timeval firstSync;              ///< @brief Stored time of first successful sync after boot
    timeval packetLastReceived;     ///< @brief Moment when a NTP response has arrived
    bool ntpRequested = false;      ///< @brief Indicates that a NTP response is pending
    unsigned long uptime = 0;       ///< @brief Time since boot
    unsigned int shortInterval = DEFAULT_NTP_SHORTINTERVAL * 1000;  ///< @brief Interval to set periodic time sync until first synchronization.
    unsigned int longInterval = DEFAULT_NTP_INTERVAL * 1000;        ///< @brief Interval to set periodic time sync
    unsigned int actualInterval = DEFAULT_NTP_SHORTINTERVAL * 1000; ///< @brief Currently selected interval
    onSyncEvent_t onSyncEvent;      ///< @brief Event handler callback
    uint16_t ntpTimeout = DEFAULT_NTP_TIMEOUT;                      ///< @brief Response timeout for NTP requests
    long minSyncAccuracyUs = DEFAULT_MIN_SYNC_ACCURACY_US;          ///< @brief DEfault minimum offset value to consider a good sync
    uint maxNumSyncRetry = DEFAULT_MAX_RESYNC_RETRY;                ///< @brief Number of resync repetitions if minimum accuracy has not been reached
    uint numSyncRetry;              ///< @brief Current resync repetition
    uint maxDispersionErrors = DEFAULT_MAX_RESYNC_RETRY;            ///< @brief Number of resync repetitions if server has a dispersion value bigger than offset absolute value
    uint numDispersionErrors;
    long timeSyncThreshold = DEFAULT_TIME_SYNC_THRESHOLD;           ///< @brief If calculated offset is below this threshold it will not be applied. 
                                                                    //            This is to avoid continious innecesary glitches in clock
    uint numTimeouts = 0;           ///< @brief After this number of timeout responses ntp sync time is increased
    NTPStatus_t status = unsyncd;   ///< @brief Sync status
    char ntpServerName[SERVER_NAME_LENGTH];                         ///< @brief  of NTP server on Internet or LAN
    IPAddress ntpServerIPAddress;   ///< @brief  IP address of NTP server on Internet or LAN
public:
#ifdef ESP32
    //bool terminateTasks = false;
    TaskHandle_t loopHandle = NULL;                                 ///< @brief TimeSync loop task handle
    TaskHandle_t receiverHandle = NULL;                             ///< @brief NTP response receiver task handle
#else
    Ticker loopTimer;               ///< @brief Timer to trigger timesync
    Ticker receiverTimer;           ///< @brief Timer to check received responses
#endif
protected:
    Ticker responseTimer;           ///< @brief Timer to trigger response timeout
    bool isConnected = false;       ///< @brief True if client has resolved correctly server IP address
    double offset;                  ///< @brief Temporary offset storage for event notify
    double delay;                   ///< @brief Temporary delay storage for event notify
    timezone timeZone;              ///< @brief 
    char tzname[TZNAME_LENGTH];     ///< @brief Configuration string for local time zone
    
    int64_t offsetSum;              ///< @brief Sum of offsets for average calculation
    int64_t offsetAve;              ///< @brief Average calculated value
    uint round = 0;                 ///< @brief Number of offset values added during last sync 
    uint numAveRounds = DEFAULT_NUM_OFFSET_AVE_ROUNDS;          ///< @brief Number of request to be done to calculate average.
    
    pbuf* lastNtpResponsePacket;    ///< @brief Last response packet to be processed by receiver task
    bool responsePacketValid = false;                           ///< @brief Is `lastNtpResponsePacket` already processed?
    
    /**
      * @brief Gets time from NTP server and convert it to Unix time format
      * @param arg `NTPClient` instance
      */
    static void s_getTimeloop (void* arg);
    
    /**
      * @brief Static method that calls `recvPacket`. Used in receiver task
      * @param arg user supplied argument (udp_pcb.recv_arg)
      * @param pcb the udp_pcb which received data
      * @param p the packet buffer that was received
      * @param addr the remote IP address from which the packet was received
      * @param port the remote port from which the packet was received
      */
    static void s_recvPacket (void* arg, struct udp_pcb* pcb, struct pbuf* p,
                              const ip_addr_t* addr, u16_t port);
    
    /**
      * @brief Receiver task to check for received packets and launch packet processor
      * @param arg `NTPClient` instance
      */ 
    static void s_receiverTask (void* arg);
    
    /**
      * @brief Checks if received packet may be used to get a good sync
      * @param ntpPacket Packet to analyze
      * @param offsetUs Microseconds offset, used to check dispersion
      * @return `true` if NTP packet is good for sync
      */
    bool checkNTPresponse (NTPPacket_t* ntpPacket, int64_t offsetUs);
    
    /**
      * @brief Write NTP packet data to Serial monitor
      * @param decPacket Packet to analyze
      */
    void dumpNtpPacketInfo (NTPPacket_t* decPacket);
    
    /**
      * @brief Static method to call NTP response timeout processor
      */
    static void s_processRequestTimeout (void* arg);
    
    /**
      * @brief Process internal state in case of a response timeout. If a response comes later is is asumed as non valid
      */
    void processRequestTimeout ();
    
    /**
      * @brief Sends NTP request to server
      * @return false in case of any error
      */
    boolean sendNTPpacket ();
    
       
    /**
      * @brief Gets packet response and update time as of its data
      * @param p UDP response packet
      */
    void processPacket (struct pbuf* p);
    
    /**
      * @brief Decodes NTP response contained in buffer
      * @param messageBuffer Pointer to message buffer
      * @param len Buffer len
      * @param decPacket Pointer to packet to store decoded data
      * @return Decoded packet from message
      */
    NTPPacket_t* decodeNtpMessage (uint8_t* messageBuffer, size_t len, NTPPacket_t* decPacket);

    /**
      * @brief Calculates offset from NTP response packet
      * @param ntpPacket NTP response packet structure
      * @return Time offset in `timeval` format
      */
    timeval calculateOffset (NTPPacket_t* ntpPacket);
    
    /**
      * @brief Applies offset to system clock
      * @param offset Calculated offset
      * @return `true` if process finished without errors
      */
    bool adjustOffset (timeval* offset);

public:
    /**
      * @brief NTP client Class destructor
      */
    ~NTPClient () {
        stop ();
    }
    
    /**
      * @brief Cleans data and stops all tasks
      */
    void stop (){
#ifdef ESP32
        if (loopHandle) {
            vTaskDelete (loopHandle);
            //DEBUGLOGI ("Loop task handle deleted");
            loopHandle = NULL;
        }
        if (receiverHandle) {
            vTaskDelete (receiverHandle);
            //DEBUGLOGI ("Receiver task handle deleted");
            receiverHandle = NULL;
        }
#else
        loopTimer.detach ();
        receiverTimer.detach ();
#endif // ESP8266
        responseTimer.detach ();
#ifdef ESP8266
        if (udp) {
            udp_remove (udp);
        }
        // if (lastNtpResponsePacket) {
        //     pbuf_free (lastNtpResponsePacket);
        // }
#endif // ESP8266
    }
    
    /**
      * @brief Starts a NTP time request to server. Only called from library, normally.
      * 
      * Kept in public section to allow direct NTP request.
      */
    void getTime ();

    /**
      * @brief Starts time synchronization
      * @param ntpServerName NTP server name as String
      * @return `true` if everything went ok
      */
    bool begin (const char* ntpServerName = DEFAULT_NTP_SERVER);
    
    /**
      * @brief Sets NTP server name
      * @param serverName New NTP server name
      * @return `true` if everything went ok
      */
    bool setNtpServerName (const char* serverName);
    
    /**
     * @brief Gets NTP server name
     * @return NTP server name
     */
    char* getNtpServerName () {
        return ntpServerName;
    }
    
    /**
      * @brief Set a callback that triggers after a sync event
      * @param handler function with `onSyncEvent_t` to notify events to user code
      */
    void onNTPSyncEvent (onSyncEvent_t handler){
        if (handler){
            onSyncEvent = handler;
        }
    }
    
    /**
      * @brief Changes sync period
      * @param interval New interval in seconds
      * @return True if everything went ok
      */
    bool setInterval (int interval);
    
    /**
      * @brief Changes sync period in sync'd and unsync'd status
      * @param shortInterval New interval while time is not first adjusted yet, in seconds
      * @param longInterval New interval for normal operation, in seconds
      * @return True if everything went ok
      */
    bool setInterval (int shortInterval, int longInterval);
    
    /**
      * @brief Gets sync period
      * @return Interval for normal operation, in seconds
      */
    int getInterval () {
        return actualInterval / 1000;
    }

    /**
      * @brief Gets sync period while not sync'd status
      * @return Interval while time is not first adjusted yet, in seconds
      */
    int	getShortInterval () {
        return shortInterval / 1000;
    }

    /**
      * @brief Gets sync period
      * @return Interval for normal operation in seconds
      */
    int	getLongInterval () { 
        return longInterval / 1000;
    }

    /**
      * @brief Sets minimum sync accuracy to get a new request if offset is greater than this value
      * @param accuracy Desired minimum accuracy
      */
    void setMinSyncAccuracy (long accuracy) {
        const int minAccuracy = 100;

        if (accuracy > minAccuracy) {
            minSyncAccuracyUs = accuracy;
        } else {
            minSyncAccuracyUs = minAccuracy;
        }
    }
    
    /**
      * @brief Sets time sync threshold. If offset is under this value time will not be adjusted
      * @param threshold Desired sync threshold
      */
    void settimeSyncThreshold (long threshold) {
        const int minThreshold = 100;
        
        if (threshold > minThreshold) {
            timeSyncThreshold = threshold;
        } else {
            timeSyncThreshold = minThreshold;
        }
    }
    
    /**
      * @brief Sets max number of sync retrials if minimum accuracy has not been reached
      * @param maxRetry Max sync retrials number
      */
    void setMaxNumSyncRetry (unsigned long maxRetry) {
            maxNumSyncRetry = maxRetry;
    }

    /**
      * @brief Configures response timeout for NTP requests
      * @param milliseconds error code. `false` in case of error
      */
    bool setNTPTimeout (uint16_t milliseconds);

    /**
      * @brief Sets time zone for getting local time
      * @param TZ Time zone description
      */
    void setTimeZone (const char* TZ){
        strncpy (tzname, TZ, TZNAME_LENGTH);
        setenv ("TZ", tzname, 1);
        tzset ();
    }
    
    /**
      * @brief Converts current time to a char string
      * @return String built from current time
      */
    char* getTimeStr () {
        time_t currentTime = time (NULL);
        return getTimeStr (currentTime);
    }

    /**
      * @brief Converts a time in UNIX format to a char string representing time
      * @param moment `timeval` object to convert to extract time
      * @return String built from given time
      */
    char* getTimeStr (timeval moment) {
        tm* local_tm = localtime (&moment.tv_usec);
        size_t index = strftime (strBuffer, sizeof (strBuffer), "%H:%M:%S", local_tm);
        snprintf (strBuffer + index, sizeof (strBuffer) - index, ".%06ld", moment.tv_usec);
        return strBuffer;
    }
    
    /**
      * @brief Converts a time in UNIX format to a char string representing time
      * @param moment `time_t` value (UNIXtime) to convert to extract time
      * @return String built from given time
      */
    char* getTimeStr (time_t moment) {
        tm* local_tm = localtime (&moment);
        strftime (strBuffer, sizeof(strBuffer), "%H:%M:%S", local_tm);
        return strBuffer;
    }

    /**
    * @brief Converts current date to a char string
    * @return String built from current date
    */
    char* getDateStr () {
        time_t currentTime = time (NULL);
        return getDateStr (currentTime);
    }

    /**
    * @brief Converts a time in UNIX format to a char string representing its date
    * @param moment `timeval` object to convert to extract date
    * @return String built from given time
    */
    char* getDateStr (timeval moment) {
        return getDateStr (moment.tv_sec);
    }
    
    /**
    * @brief Converts a time in UNIX format to a char string representing its date
    * @param moment time_t object to convert to extract date
    * @return String built from given time
    */
    char* getDateStr (time_t moment) {
        tm* local_tm = localtime (&moment);
        strftime (strBuffer, sizeof (strBuffer), "%02d/%m/%04Y", local_tm);
        return strBuffer;
    }
    
    /**
    * @brief Converts current time and date to a char string
    * @param[out] Char string built from current time.
    */
    char* getTimeDateString () {
        time_t currentTime = time (NULL);
        return getTimeDateString (currentTime);
    }

    /**
    * @brief Converts current time and date to a char string
    * @return Char string built from current time
    */
    char* getTimeDateStringUs () {
        timeval currentTime;
        gettimeofday (&currentTime, NULL);
        return getTimeDateString (currentTime);
    }
    
    /**
    * @brief Converts current time and date to a char string
    * @return Char string built from current time
    */
    char* getTimeDateStringForJS () {
        return getTimeDateString (time (NULL), "%02m/%02d/%04Y %02H:%02M:%02S");
    }
    
    /**
    * @brief Converts given time and date to a char string
    * @param moment `timeval` object to convert to String
    * @param format Format as printf
    * @return Char string built from current time
    */
    char* getTimeDateString (timeval moment, const char* format = "%02d/%02m/%04Y %02H:%02M:%02S") {
        tm* local_tm = localtime (&moment.tv_sec);
        size_t index = strftime (strBuffer, sizeof (strBuffer), format, local_tm);
        index += snprintf (strBuffer + index, sizeof (strBuffer) - index, ".%06ld", moment.tv_usec);
        strftime (strBuffer + index, sizeof (strBuffer) - index, " %Z", local_tm);
        return strBuffer;
    }

    /**
    * @brief Converts given time and date to a char string
    * @param moment `time_t` value (UNIX time) to convert to char string
    * @param format Format as printf
    * @return Char string built from current time
    */
    char* getTimeDateString (time_t moment, const char* format = "%02d/%02m/%04Y %02H:%02M:%02S") {
        tm* local_tm = localtime (&moment);
        strftime (strBuffer, sizeof (strBuffer), format, local_tm);

        return strBuffer;
    }
    
    /**
    * @brief Gets last successful sync time in UNIX format, with microseconds
    * @return Last successful sync time. 0 equals never
    */
    timeval getLastNTPSyncUs () {
        return lastSyncd;
    }

    
    /**
    * @brief Gets last successful sync time in UNIX format
    * @return Last successful sync time. 0 equals never
    */
    time_t getLastNTPSync (){
        return lastSyncd.tv_sec;
    }

    /**
    * @brief Gets uptime in human readable String format
    * @return Uptime
    */
    char* getUptimeString ();

    /**
    * @brief Gets uptime in UNIX format, time since MCU was last rebooted
    * @return Uptime
    */
    time_t getUptime () {
        uptime = uptime + (::millis () - uptime);
        return uptime / 1000;
    }

    /**
    * @brief Gets first successful synchronization time after boot
    * @return First sync time
    */
    timeval getFirstSyncUs () {
        return firstSync;
    }
    
    /**
    * @brief Gets first successful synchronization time after boot
    * @return First sync time
    */
    time_t getFirstSync () {
        return firstSync.tv_sec;
    }
    
    /**
    * @brief Returns sync status
    * @return Sync status as NTPStatus_t
    */
    NTPStatus_t syncStatus () {
        return status;
    }

    /**
     * @brief Gets milliseconds since 1-Jan-1970 00:00 UTC
     * @return Milliseconds since 1-Jan-1970 00:00 UTC
     */
    int64_t millis () {
        timeval currentTime;
        gettimeofday (&currentTime, NULL);
        int64_t milliseconds = (int64_t)currentTime.tv_sec * 1000L + (int64_t)currentTime.tv_usec / 1000L;
        //Serial.printf ("timeval: %ld.%ld millis %lld\n", currentTime.tv_sec, currentTime.tv_usec, milliseconds);
        return milliseconds;
    }
    
    /**
     * @brief Gets microseconds since 1-Jan-1970 00:00 UTC
     * @return microseconds since 1-Jan-1970 00:00 UTC
     */
    int64_t micros() {
        timeval currentTime;
        gettimeofday (&currentTime, NULL);
        int64_t microseconds = (int64_t)currentTime.tv_sec * 1000000L + (int64_t)currentTime.tv_usec;
        //Serial.printf ("timeval: %ld.%ld micros %lld\n", currentTime.tv_sec, currentTime.tv_usec, microseconds);
        return microseconds;
    }

    /**
     * @brief Gets text description from error. Useful for debugging
     * @param e NTP event
     * @return Text description
     */
    char* ntpEvent2str (NTPEvent_t e);

    /**
     * @brief Sets the number of sync attempts to calculate average offset
     * @param rounds Number of average rounds 1.. MAX_OFFSET_AVERAGE_ROUNDS
     */
    void setnumAveRounds (int rounds) {
        if (rounds < 1) {
            numAveRounds = 1;
        } else if (rounds > MAX_OFFSET_AVERAGE_ROUNDS) {
            numAveRounds = MAX_OFFSET_AVERAGE_ROUNDS;
        } else {
            numAveRounds = rounds;
        }
    }

    /**
     * @brief Gets the number of sync attempts to calculate average offset
     * @return Number of average rounds 1.. MAX_OFFSET_AVERAGE_ROUNDS
     */
    uint getnumAveRounds () {
        return numAveRounds;
    }
    
};

extern NTPClient NTP; ///< @brief Singleton NTPClient instance

#endif // _NtpClientLib_h
