#include <Arduino.h>

//#include "WifiConfig.h"

#include <ESPNtpClient.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "YOUR_WIFI_SSID"
#define YOUR_WIFI_PASSWD "YOUR_WIFI_PASSWD"
#endif // !WIFI_CONFIG_H

const PROGMEM char* ntpServer = "pool.ntp.org";

#ifndef LED_BUILTIN
#define LED_BUILTIN 5
#endif

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPEvent_t ntpEvent; // Last triggered event

void processSyncEvent (NTPEvent_t ntpEvent) {
    Serial.printf ("[NTP-event] %s\n", NTP.ntpEvent2str(ntpEvent));
}


void setup () {
    Serial.begin (115200);
    Serial.println ();
    WiFi.begin (YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
    NTP.setTimeZone (TZ_Etc_UTC);
    NTP.onNTPSyncEvent ([] (NTPEvent_t event) {
        ntpEvent = event;
        syncEventTriggered = true;
    });
    NTP.setInterval (300);
    NTP.begin (ntpServer);
    pinMode (LED_BUILTIN, OUTPUT);
    digitalWrite (LED_BUILTIN, HIGH);
}

void loop () {
    timeval currentTime;
    gettimeofday (&currentTime, NULL);
    int64_t us = NTP.micros () % 1000000L;
    digitalWrite (LED_BUILTIN, !((us >= 0 && us < 10000) || (us >= 150000 && us < 160000)));
    if (syncEventTriggered) {
        syncEventTriggered = false;
        processSyncEvent (ntpEvent);
    }
}
