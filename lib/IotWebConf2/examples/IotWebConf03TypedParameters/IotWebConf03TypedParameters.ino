/**
 * IotWebConf03TypedParameters.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Custom parameters
 * Description:
 *   Typed Parameters are a new approach to store/handle parameter data.
 *   This part of the software is very experimental, and certainly
 *   not recommended for beginners.
 *   The goal of this particular example is to compare the original
 *   approach of IotWebConf03CustomParameters to this new typed
 *   parameters, as both examples should work the same.
 *
 * Hardware setup for this example:
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - [Optional] A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */

#include <IotWebConf.h>
#include <IotWebConfTParameter.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "dem3"

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
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);

DNSServer dnsServer;
WebServer server(80);

static const char chooserValues[][STRING_LEN] = { "red", "blue", "darkYellow" };
static const char chooserNames[][STRING_LEN] = { "Red", "Blue", "Dark yellow" };

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
iotwebconf::TextTParameter<STRING_LEN> stringParam =
  iotwebconf::Builder<iotwebconf::TextTParameter<STRING_LEN>>("stringParam").
  label("String param").build();
iotwebconf::ParameterGroup group1 = iotwebconf::ParameterGroup("group1", "");
iotwebconf::IntTParameter<int16_t> intParam =
  iotwebconf::Builder<iotwebconf::IntTParameter<int16_t>>("intParam").
  label("Int param").
  defaultValue(30).
  min(1).max(100).
  step(1).placeholder("1..100").build();
// -- We can add a legend to the separator
iotwebconf::ParameterGroup group2 = iotwebconf::ParameterGroup("c_factor", "Calibration factor");
iotwebconf::FloatTParameter floatParam =
   iotwebconf::Builder<iotwebconf::FloatTParameter>("floatParam").
   label("Float param").
   defaultValue(0.0).
   step(0.1).placeholder("e.g. 23.4").build();
iotwebconf::CheckboxTParameter checkboxParam =
   iotwebconf::Builder<iotwebconf::CheckboxTParameter>("checkParam").
   label("Check param").
   defaultValue(true).
   build();
iotwebconf::SelectTParameter<STRING_LEN> chooserParam =
   iotwebconf::Builder<iotwebconf::SelectTParameter<STRING_LEN>>("chooseParam").
   label("Choose param").
   optionValues((const char*)chooserValues).
   optionNames((const char*)chooserNames).
   optionCount(sizeof(chooserValues) / STRING_LEN).
   nameLength(STRING_LEN).
   build();


void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  group1.addItem(&intParam);
  group2.addItem(&floatParam);
  group2.addItem(&checkboxParam);
  group2.addItem(&chooserParam);

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addSystemParameter(&stringParam);
  iotWebConf.addParameterGroup(&group1);
  iotWebConf.addParameterGroup(&group2);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.getApTimeoutParameter()->visible = true;

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
  s += "<title>IotWebConf 03 Custom Parameters</title></head><body>Hello world!";
  s += "<ul>";
  s += "<li>String param value: ";
  s += stringParam.value();
  s += "<li>Int param value: ";
  s += intParam.value();
  s += "<li>Float param value: ";
  s += floatParam.value();
  s += "<li>CheckBox selected: ";
  s += checkboxParam.isChecked();
  s += "<li>Option selected: ";
  s += chooserParam.value();
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
  Serial.println("Validating form.");
  bool valid = true;

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

