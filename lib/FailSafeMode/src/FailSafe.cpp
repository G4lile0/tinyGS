/**
  * @file FailSafe.cpp
  * @version 0.2.3
  * @date 18/12/2020
  * @author German Martin
  * @brief Library to add a simple fail safe mode to any ESP32 or ESP8266 project
  */

#include "FailSafe.h"

#if defined ESP8266
#include "coredecls.h"
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#endif

#if defined ESP32 || FS_USE_FLASH
  #include "FS.h"
  #if defined ESP8266
    #if FS_USE_LITTLEFS
        #define FAILSAFE_FS LittleFS
        #include <LittleFS.h>
    #else
        #define FAILSAFE_FS SPIFFS
    #endif // FS_USE_LITTLEFS
  #else
    #include <SPIFFS.h>
    #define FAILSAFE_FS SPIFFS
  #endif // ESP8266
#endif // ESP32 || FS_USE_FLASH

#include <ArduinoOTA.h>

#if FAIL_SAFE_DEBUG
const char* IRAM_ATTR extractFileName (const char* path) {
    size_t i = 0;
    size_t pos = 0;
    char* p = (char*)path;
    while (*p) {
        i++;
        if (*p == '/' || *p == '\\') {
            pos = i;
        }
        p++;
    }
    return path + pos;
}
#endif // FAIL_SAFE_DEBUG

void FailSafeClass::resetFlag () {
    fsDebug ("Reset flag");
    bootFlag.bootCycles = 0;
    saveFlag ();
}

const char* FailSafeClass::toString () {
    switch (failSafe) {
    case OFF:
        return "OFF";
    case TRIGGERED:
        return "TRIGGERED";
    case RUNNING:
        return "RUNNING";
    }
    return "";
}

#if defined ESP8266 && !FS_USE_FLASH
bool FailSafeClass::loadFlag () {
    uint32_t crcCheck;

    if (ESP.rtcUserMemoryRead (offset, (uint32_t*)&bootFlag, sizeof (bootFlag_t))) {
        fsDebug ("Read RTCData: %08X CRC: %08X", bootFlag.bootCycles, bootFlag.crc);
    } else {
        fsDebug ("Error reading RTC memory");
        return false;
    }

    crcCheck = crc32 ((uint8_t*)&(bootFlag.bootCycles), sizeof (int32_t));
    fsDebug ("Calculated CRC: %08X\n", crcCheck);
    if (crcCheck == bootFlag.crc) {
        fsDebug ("loadFlag: CRC OK");
        return true;
    } else {
        resetFlag ();
        fsDebug ("loadFlag: CRC Error");
    }
    return false;
}
#elif defined ESP32 || FS_USE_FLASH
bool FailSafeClass::loadFlag () {
    if (!FAILSAFE_FS.begin ()) {
        FAILSAFE_FS.format ();
        resetFlag ();
        return false;
    }
    
    fsDebug ("Started Filesystem for loading");

    if (!FAILSAFE_FS.exists (FILENAME)) {
        fsDebug ("File %s not found", FILENAME);
        return true;
    }

    fsDebug ("Opening %s file", FILENAME);
    File configFile = FAILSAFE_FS.open (FILENAME, "r");
    if (!configFile) {
        fsDebug ("Error opening %s", FILENAME);
        resetFlag ();
        return false;
    }
        
    fsDebug ("%s opened", FILENAME);
    size_t size = configFile.size ();

    if (size < sizeof (bootFlag_t)) {
        fsDebug ("File size error");
        return false;
    }

    configFile.read ((uint8_t*)&bootFlag, sizeof (bootFlag));
    configFile.close ();

    fsDebug ("Read RTCData: %08X", bootFlag.bootCycles);

    return true;
}
#endif

#if defined ESP8266 && !FS_USE_FLASH
bool FailSafeClass::saveFlag () {
    bootFlag.crc = crc32 ((uint8_t*)&(bootFlag.bootCycles), sizeof (int32_t));

    if (ESP.rtcUserMemoryWrite (offset, (uint32_t*)&bootFlag, sizeof (bootFlag_t))) {
        fsDebug ("Write RTCData: %d CRC: %08X", bootFlag.bootCycles, bootFlag.crc);
        return true;
    } else {
        fsDebug ("Error writting RTC memory");
    }
    return false;
}
#elif defined ESP32 || FS_USE_FLASH
bool FailSafeClass::saveFlag () {
    //if (!FAILSAFE_FS.begin ()) {
    //    fsDebug ("Error opening FS for saving");
    //    return false;
    //}    

    fsDebug ("Started Filesystem for saving");

    File configFile = FAILSAFE_FS.open (FILENAME, "w");
    if (!configFile) {
        fsDebug ("failed to open config file %s for writing", FILENAME);
        return false;
    }

    configFile.write ((uint8_t*)&bootFlag, sizeof (bootFlag));
    configFile.flush ();
    configFile.close ();

    fsDebug ("Write FSData: %d", bootFlag.bootCycles);

    return true;
}
#endif

void FailSafeClass::checkBoot (int maxBootCycles, int led, uint32_t memOffset) {
    offset = memOffset;
    indicatorLed = led;

    pinMode (indicatorLed, OUTPUT);
    digitalWrite (indicatorLed, HIGH);

    if (loadFlag ()) {
        if (bootFlag.bootCycles >= maxBootCycles) {
            failSafe = TRIGGERED;
        } else {
            bootFlag.bootCycles++;
            saveFlag ();
        }
    }
    fsDebug ("Fail boot is %s", toString ());
}

void FailSafeClass::failSafeModeSetup () {
    digitalWrite (indicatorLed, LOW);
    WiFi.disconnect ();
    WiFi.mode (WIFI_AP);
    char bssid[30];
#if defined ESP8266
    snprintf (bssid, 30, "ESP_%06X_failsafe", ESP.getChipId ());
#elif defined ESP32
    snprintf (bssid, 30, "esp_%06x_failsafe", (uint32_t)(ESP.getEfuseMac () & (uint64_t)0x0000000000FFFFFF));
#endif

    WiFi.softAP (bssid, PASSWD);

    fsDebug ("AP started - %s - IP: %s\n", bssid, WiFi.softAPIP ().toString ().c_str ());

    //if (!MDNS.begin (bssid)) {
    //    Serial.println ("Error setting up MDNS responder!");
    //    while (1) {
    //        delay (1000);
    //    }
    //}
    //Serial.println ("mDNS responder started");
    //MDNS.enableWorkstation (ESP_IF_WIFI_AP);

    int led = indicatorLed;

    ArduinoOTA.onStart ([]() {
        fsDebug ("OTA Start");
                        });
    ArduinoOTA.onProgress ([led](unsigned int progress, unsigned int total) {
#define INIT_PERIOD 500
#define PULSE_TIME 10

        static time_t lastFlash;
        float ratio = 1 - (float)progress / (float)total;
        unsigned int period = INIT_PERIOD * ratio + PULSE_TIME + 10;
        int state = digitalRead (led) == LOW;
        //fsDebug ("ratio: %f period: %d state: %d\n", ratio, period, state);
        if (!state) { // LED off
            if (millis () - lastFlash > period) {
                
                lastFlash = millis ();
                digitalWrite (led, LOW);
#if FAIL_SAFE_DEBUG
                Serial.print (".");
#endif
            }
        } else { // LED on
            if (millis () - lastFlash > PULSE_TIME) {
                digitalWrite (led, HIGH);
            }
        }
                           });
    ArduinoOTA.onEnd ([led]() {
        digitalWrite (led, LOW);
        fsDebug ("OTA End");
                      });
    ArduinoOTA.onError ([led](ota_error_t error) {
        digitalWrite (led, HIGH);
        fsDebug ("OTA Error %d", error);
                        });

    ArduinoOTA.setHostname (bssid);
    //ArduinoOTA.setPassword (PASSWD);

    ArduinoOTA.begin ();
}

void FailSafeClass::failSafeModeLoop () {
    ArduinoOTA.handle ();
}


void FailSafeClass::loop (uint32_t timeout) {
    if (bootFlag.bootCycles > 0) {
        if (millis () > timeout) {
            fsDebug ("Flag timeout %d", millis ());
            resetFlag ();
        }
    }

    if (failSafe == TRIGGERED) {
        failSafe = RUNNING;
        fsDebug ("Fail Safe mode started");
        failSafeModeSetup ();
        fsDebug ("Mode is now %s", FailSafe.toString ());
    }

    if (failSafe == RUNNING) {
        failSafeModeLoop ();
    }

}


FailSafeClass FailSafe;

