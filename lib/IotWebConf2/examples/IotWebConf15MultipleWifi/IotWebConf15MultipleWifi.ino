/**
 * IotWebConf15MultipleWifi.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2021 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Multiple WiFi
 * Description:
 *   In this example we are setting up config portal, so that
 *   admins can provide more than one WiFi connection info.
 *   The idea is, that if one connection is not available, then
 *   the next one will be tried.
 *   The MultipleWifiAddition registers all required component into
 *   IotWebConf. Note, that both formValidator and htmlFormatProvider
 *   of IotWebConf are set with calling MultipleWifiAddition.init().
 *   Also note, that chainedWifiParameterGroups[] should be prefilled
 *   with ChainedWifiParameterGroup instances, as we would like to
 *   avoid dynamic memory allocations.
 *
 * Hardware setup for this example:
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - [Optional] A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */

#include <IotWebConf.h>
#include <IotWebConfMultipleWifi.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "opt4"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN D2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Method declarations.
void handleRoot();
// -- Callback methods.
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);

DNSServer dnsServer;
WebServer server(80);

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

iotwebconf::ChainedWifiParameterGroup chainedWifiParameterGroups[] = {
  iotwebconf::ChainedWifiParameterGroup("wifi1"),
  iotwebconf::ChainedWifiParameterGroup("wifi2"),
  iotwebconf::ChainedWifiParameterGroup("wifi3")
};

iotwebconf::MultipleWifiAddition multipleWifiAddition(
  &iotWebConf,
  chainedWifiParameterGroups,
  sizeof(chainedWifiParameterGroups)  / sizeof(chainedWifiParameterGroups[0]));

iotwebconf::OptionalGroupHtmlFormatProvider optionalGroupHtmlFormatProvider;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);

  // -- Initializing the configuration.
  multipleWifiAddition.init();
  // -- Note: multipleWifiAddition.init() calls setFormValidator, that
  // overwrites existing formValidator setup. Thus setFormValidator
  // should be called _after_ multipleWifiAddition.init() .
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setHtmlFormatProvider(&optionalGroupHtmlFormatProvider);
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
  s += "<title>IotWebConf 14 Group chain</title></head><body><div>Status page of ";
  s += iotWebConf.getThingName();
  s += ".</div>";

/*
  ActionGroup* group = &actionGroup1;
  while(group != NULL)
  {
    if (group->isActive())
    {
      s += "<div>Action:</div>";
      s += "<ul>";
      s += "<li>Action type: ";
      s += group->actionTypeValue;
      s += "<li>MQTT topic: ";
      s += group->mqttTopicValue;
      s += "<li>MQTT message pattern: ";
      s += group->mqttMsgPatternValue;
      s += "<li>Delay (secs): ";
      s += atoi(group->delaySecsValue);
      s += "</ul>";
    }
    group = (ActionGroup*)group->getNext();
  }
*/

  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}


bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
  Serial.println("Validating form.");
  // -- Note: multipleWifiAddition.formValidator() should be called, as
  // we have override this setup.
  bool valid = multipleWifiAddition.formValidator(webRequestWrapper);

/*
  int l = webRequestWrapper->arg(stringParam.getId()).length();
  if (l < 3)
  {
    stringParam.errorMessage = "Please provide at least 3 characters for this test!";
    valid = false;
  }
*/
  return valid;
}
