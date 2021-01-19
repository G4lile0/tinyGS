/**
 * IotWebConf08WebRelay.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Web Relay Demo
 * Description:
 *   All IotWebConf specific aspects of this example are described in
 *   previous examples, so please get familiar with IotWebConf before
 *   starting this example. So nothing new will be explained here, 
 *   but a complete demo application will be built.
 *   It is also expected from the reader to have a basic knowledge over
 *   HTML to understand this code.
 *   As an addition an example is show here with combining the chip-ID
 *   into the initial thing name.
 *   
 *   This example stets up a web page, where switch action can take place.
 *   The repay can be also altered with the push button. 
 *   The thing will delay actions arriving within 10 seconds.
 *   
 *   This example also provides the firmware update option.
 *   (See previous examples for more details!)
 * 
 * Hardware setup for this example:
 *   - A Relay is attached to the D1 pin (On=HIGH)
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */

#include <IotWebConf.h>
#ifdef ESP8266
# include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
# include <IotWebConfESP32HTTPUpdateServer.h>
#endif

#ifdef ESP8266
String ChipId = String(ESP.getChipId(), HEX);
#elif ESP32
String ChipId = String((uint32_t)ESP.getEfuseMac(), HEX);
#endif

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
String thingName = String("testThing-") + ChipId;

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define STRING_LEN 128

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "web1"

// -- When BUTTON_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define BUTTON_PIN D2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Connected ouput pin.
#define RELAY_PIN D1

#define ACTION_FEQ_LIMIT 10000
#define NO_ACTION -1

// -- Method declarations.
void handleRoot();
void applyAction(unsigned long now);
// -- Callback methods.
void configSaved();

DNSServer dnsServer;
WebServer server(80);
#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
HTTPUpdateServer httpUpdater;
#endif

IotWebConf iotWebConf(thingName.c_str(), &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

bool needReset = false;
int needAction = NO_ACTION;
int state = LOW;
unsigned long lastAction = 0;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  pinMode(RELAY_PIN, OUTPUT);

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(BUTTON_PIN);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });

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

  if (needReset)
  {
    Serial.println("Rebooting after 1 second.");
    iotWebConf.delay(1000);
    ESP.restart();
  }

  unsigned long now = millis();

  // -- Check for button push
  if ((digitalRead(BUTTON_PIN) == LOW)
    && (ACTION_FEQ_LIMIT < now - lastAction))
  {
    needAction = 1 - state; // -- Invert the state
  }

  applyAction(now);
}

void applyAction(unsigned long now)
{
  if ((needAction != NO_ACTION)
    && (ACTION_FEQ_LIMIT < now - lastAction))
  {
    state = needAction;
    digitalWrite(RELAY_PIN, state);
    if (state == HIGH)
    {
      iotWebConf.blink(5000, 95);
    }
    else
    {
      iotWebConf.stopCustomBlink();
    }
    Serial.print("Switched ");
    Serial.println(state == HIGH ? "ON" : "OFF");
    needAction = NO_ACTION;
    lastAction = now;
  }
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

  if (server.hasArg("action"))
  {
    String action = server.arg("action");
    if (action.equals("on"))
    {
      needAction = HIGH;
    }
    else if (action.equals("off"))
    {
      needAction = LOW;
    }
    applyAction(millis());
  }
  
  String s = F("<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>");
  s += iotWebConf.getHtmlFormatProvider()->getStyle();
  s += "<title>IotWebConf 08 Web Relay</title></head><body>";
  s += iotWebConf.getThingName();
  s += "<div>State: ";
  s += (state == HIGH ? "ON" : "OFF");
  s += "</div>";
  s += "<div>";
  s += "<button type='button' onclick=\"location.href='?action=on';\" >Turn ON</button>";
  s += "<button type='button' onclick=\"location.href='?action=off';\" >Turn OFF</button>";
  s += "<button type='button' onclick=\"location.href='?';\" >Refresh</button>";
  s += "</div>";
  s += "<div>Go to <a href='config'>configure page</a> to change values.</div>";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
  needReset = true;
}
