/**
 * IotWebConf01Minimal.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Minimal
 * Description:
 *   This example will shows the bare minimum required for IotWebConf to start up.
 *   After starting up the thing, please search for WiFi access points e.g. with
 *   your phone. Use password provided in the code!
 *   After connecting to the access point the root page will automatically appears.
 *   We call this "captive portal".
 *   
 *   Please set a new password for the Thing (for the access point) as well as
 *   the SSID and password of your local WiFi. You cannot move on without these steps.
 *   
 *   You have to leave the access point before to let the Thing continue operation
 *   with connecting to configured WiFi.
 *
 *   Note that you can find detailed debug information in the serial console depending
 *   on the settings IOTWEBCONF_DEBUG_TO_SERIAL, IOTWEBCONF_DEBUG_PWD_TO_SERIAL set
 *   in the IotWebConf.h .
 */

#include <IotWebConf.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

// -- Method declarations.
void handleRoot();

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  // -- Initializing the configuration.
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  Serial.println("Ready.");
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 01 Minimal</title></head><body>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

