#ifndef _NtpEventTypes_h
#define _NtpEventTypes_h

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

/**
  * @brief NTP event codes
  */
typedef enum {
    timeSyncd = 0, /**< Time successfully got from NTP server */
    noResponse = -1, /**< No response from server */
    invalidAddress = -2, /**< Address not reachable */
    invalidPort = -3, /**< Port already used */
    requestSent = 1, /**< NTP request sent, waiting for response */
    partlySync = 2, /**< Successful sync but offset was over threshold */
    syncNotNeeded = 3, /**< Successful sync but offset was under minimum threshold */
    errorSending = -4, /**< An error happened while sending the request */
    responseError = -5, /**< Wrong response received */
    syncError = -6, /**< Error adjusting time */
    accuracyError = -7 /**< NTP server time is not accurate enough */
} NTPSyncEventType_t;

/**
  * @brief NTP event info
  */
typedef struct {
    double offset = 0.0; /**< Last offset applied */
    double delay = 0.0; /**< Last calculates round trip delay to NTP server */
    float dispersion = 0.0;
    IPAddress serverAddress; /**< NTP server IP address */
    unsigned int port = 0; /**< NTP port used */
    unsigned int retrials = 0; /**< Number of resync retrials until time was got with required accuracy */
} NTPSyncEventInfo_t;

/**
  * @brief NTP Event
  */
typedef struct {
    NTPSyncEventType_t event; /**< Event code */
    NTPSyncEventInfo_t info; /**< Event related information */
} NTPEvent_t;

#endif // _NtpEventTypes_h