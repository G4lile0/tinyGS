/**
 * IotWebConf14GroupChain.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Parameter Group
 * Description:
 *   In this example we are building up a list of groups. (Remember
 *   that a group is a set of property items, rendered fieldset
 *   in the config portal.) The idea here is that admins can add
 *   multiple sets, one after another.
 *   For the group (where we collect a set of properties) we create
 *   a new class with name ActionGroup. We must create the maximal
 *   amount of ActionGroup instances should be needed. The group
 *   instances are now can be organized in a chain with the help of
 *   the setNext() method.
 *   Note, that upon update, property values will not be erased
 *   in inactive groups, what you need to test is, that the
 *   group.isActive() or not.
 * 
 * Hardware setup for this example:
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - [Optional] A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */

#include <IotWebConf.h>
#include <IotWebConfOptionalGroup.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "opt2"

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

DNSServer dnsServer;
WebServer server(80);

static char actionTypeValues[][STRING_LEN] = { "down", "up" };
static char actionTypeNames[][STRING_LEN] = { "Move UP", "Move DOWN" };

class ActionGroup : public iotwebconf::ChainedParameterGroup
{
public:
  ActionGroup(const char* id) : ChainedParameterGroup(id, "Action")
  {
    // -- Update parameter Ids to have unique ID for all parameters within the application.
    snprintf(actionTypeId, STRING_LEN, "%s-at", this->getId());
    snprintf(mqttTopicId, STRING_LEN, "%s-topic", this->getId());
    snprintf(mqttMsgPatternId, STRING_LEN, "%s-msg", this->getId());
    snprintf(delaySecsId, STRING_LEN, "%s-offs", this->getId());

    // -- Add parameters to this group.
    this->addItem(&this->actionTypeParam);
    this->addItem(&this->mqttTopicParam);
    this->addItem(&this->mqttMsgPatternParam);
    this->addItem(&this->delaySecsParam);
  }

  char actionTypeValue[STRING_LEN];
  char mqttTopicValue[STRING_LEN];
  char mqttMsgPatternValue[STRING_LEN];
  char delaySecsValue[NUMBER_LEN];
  iotwebconf::SelectParameter actionTypeParam =
    iotwebconf::SelectParameter("Action type", actionTypeId, actionTypeValue, STRING_LEN, (char*)actionTypeValues, (char*)actionTypeNames, sizeof(actionTypeValues) / STRING_LEN, STRING_LEN);
  iotwebconf::TextParameter mqttTopicParam =
    iotwebconf::TextParameter("MQTT Topics", mqttTopicId, mqttTopicValue, STRING_LEN);
  iotwebconf::TextParameter mqttMsgPatternParam =
    iotwebconf::TextParameter("MQTT Message pattern", mqttMsgPatternId, mqttMsgPatternValue, STRING_LEN);
  iotwebconf::NumberParameter delaySecsParam =
    iotwebconf::NumberParameter("Delay (secs)", delaySecsId, delaySecsValue, NUMBER_LEN, "0", "0..1200", "min='0' max='1200' step='1'");
private:
  char actionTypeId[STRING_LEN];
  char mqttTopicId[STRING_LEN];
  char mqttMsgPatternId[STRING_LEN];
  char delaySecsId[STRING_LEN];
};

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

// -- We need to declare an instance for all ActionGroup items, that can
//    appear in the config portal
ActionGroup actionGroup1 = ActionGroup("ag1");
ActionGroup actionGroup2 = ActionGroup("ag2");
ActionGroup actionGroup3 = ActionGroup("ag3");
ActionGroup actionGroup4 = ActionGroup("ag4");

// -- An instance must be created from the class defined above.
iotwebconf::OptionalGroupHtmlFormatProvider optionalGroupHtmlFormatProvider;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  actionGroup1.setNext(&actionGroup2);
  actionGroup2.setNext(&actionGroup3);
  actionGroup3.setNext(&actionGroup4);

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.setHtmlFormatProvider(&optionalGroupHtmlFormatProvider);
  // We also need to add all these parameter groups to iotWebConf
  iotWebConf.addParameterGroup(&actionGroup1);
  iotWebConf.addParameterGroup(&actionGroup2);
  iotWebConf.addParameterGroup(&actionGroup3);
  iotWebConf.addParameterGroup(&actionGroup4);
  iotWebConf.setConfigSavedCallback(&configSaved);

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
  s += "<title>IotWebConf 14 Group chain</title></head><body><div>Status page of ";
  s += iotWebConf.getThingName();
  s += ".</div>";

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

  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
}
