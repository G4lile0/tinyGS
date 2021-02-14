/**
  * @file FailSafeMode.ino
  * @version 0.2.3
  * @date 18/12/2020
  * @author German Martin
  * @brief Example on how to use library to add Fail Safe mode to any ESP32 or ESP8266 project
  */

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#if defined ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#endif

#include <FailSafe.h>

#ifdef ESP32
#define LED_BUILTIN 5
#endif

const time_t BOOT_FLAG_TIMEOUT = 10000; // Time in ms to reset flag
const int MAX_CONSECUTIVE_BOOT = 3; // Number of rapid boot cycles before enabling fail safe mode
const int LED = LED_BUILTIN; // Number of rapid boot cycles before enabling fail safe mode
const int RTC_ADDRESS = 0; // If you use RTC memory adjust offset to not overwrite other data

const char* WIFI_SSID = "...";
const char* WIFI_PASS = "...";

void setup () {
    Serial.begin (115200);
    FailSafe.checkBoot (MAX_CONSECUTIVE_BOOT, LED, RTC_ADDRESS); // Parameters are optional
    if (FailSafe.isActive ()) { // Skip all user setup if fail safe mode is activated
        return;
    }
    // --------------------------------------------------------------------
    // Insert normal setup code after here
    // Example code below
    WiFi.mode (WIFI_STA);
    WiFi.begin (WIFI_SSID, WIFI_PASS);
    time_t startConnect = millis ();
    while (!WiFi.isConnected ()) {
        if (millis () - startConnect > 10000) {
            // Force failSafe mode if WiFi connection is unsuccessful after 10 seconds
            FailSafe.startFailSafe ();
            break;
        } else {
            Serial.print ('.');
        }
        delay (250);
    }
    if (WiFi.isConnected ()) {
        Serial.printf ("WiFi connected. IP: %s\n", WiFi.localIP ().toString ().c_str ());
    }
    // --------------------------------------------------------------------
}

void loop () {

    FailSafe.loop (BOOT_FLAG_TIMEOUT); // Use always this line

    if (FailSafe.isActive ()) { // Skip all user loop code if Fail Safe mode is active
        return;
    }

    // --------------------------------------------------------------------
    
    
    // Put here your main code


    // --------------------------------------------------------------------

    //*******************************************************************************************/
    //              TEST CODE. ALL THIS CAN BE DELETED
    // Enable one option for testing

//#define FORCE_ERROR
//#define START_MANUALLY

#ifdef FORCE_ERROR // Only for testing library
    // Force program error
    int a;
    for (int i = 5; i > -1; i--) {
        Serial.printf ("%d %d - ", i, a);
        a = 10 / i; // Division by zero!!!
        delay (250);
    }
#endif // FORCE_ERROR

#ifdef START_MANUALLY // Only for testing library
    // Start FailSafe Mode manually after 20 seconds
    if (millis () > 20000) {
        Serial.println ("Start fail safe mode");
        FailSafe.startFailSafe ();
    }
#endif // FORCE_ERROR
    //              TEST CODE. ALL THIS CAN BE DELETED
    //*******************************************************************************************/
}
