/**
 * IotWebConf07MqttRelay.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: MQTT Relay Demo
 * Description:
 *   All IotWebConf specific aspects of this example are described in
 *   previous examples, so please get familiar with IotWebConf before
 *   starting this example. So nothing new will be explained here, 
 *   but a complete demo application will be built.
 *   It is also expected from the reader to have a basic knowledge over
 *   MQTT to understand this code.
 *   
 *   This example starts an MQTT client with the configured
 *   connection settings.
 *   Will receives messages appears in channel "/devices/[thingName]/action"
 *   with payload ON/OFF, and reports current state in channel
 *   "/devices/[thingName]/status" (ON/OFF). Where the thingName can be
 *   configured in the portal. A relay will be switched on/off
 *   corresponding to the received action. The relay can be also controlled
 *   by the push button.
 *   The thing will delay actions arriving within 7 seconds.
 *   
 *   This example also provides the firmware update option.
 *   (See previous examples for more details!)
 * 
 * Software setup for this example:
 *   This example utilizes Joel Gaehwiler's MQTT library.
 *   https://github.com/256dpi/arduino-mqtt
 * 
 * Hardware setup for this example:
 *   - A Relay is attached to the D5 pin (On=HIGH). Note on relay pin!
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 *
 * Note on relay pin
 *   Some people might want to use Wemos Relay Shield to test this example.
 *   Now Wemos Relay Shield connects the relay to pin D1.
 *   However, when using D1 as output, Serial communication will be blocked.
 *   So you will either keep on using D1 and miss the Serial monitor
 *   feedback, or connect your relay to another digital pin (e.g. D5).
 *   (You can modify your Wemos Relay Shield for that, as I show it in this
 *   video: https://youtu.be/GykA_7QmoXE)
 */

#include <MQTT.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.
#ifdef ESP8266
# include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
# include <IotWebConfESP32HTTPUpdateServer.h>
#endif

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define STRING_LEN 128

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "mqt2"

// -- When BUTTON_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define BUTTON_PIN D2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Connected ouput pin. See "Note on relay pin"!
#define RELAY_PIN D5

#define MQTT_TOPIC_PREFIX "/devices/"

// -- Ignore/limit status changes more frequent than the value below (milliseconds).
#define ACTION_FEQ_LIMIT 7000
#define NO_ACTION -1

// -- Method declarations.
void handleRoot();
void mqttMessageReceived(String &topic, String &payload);
bool connectMqtt();
// -- Callback methods.
void wifiConnected();
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);

DNSServer dnsServer;
WebServer server(80);
#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
HTTPUpdateServer httpUpdater;
#endif
WiFiClient net;
MQTTClient mqttClient;

char mqttServerValue[STRING_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
// -- You can also use namespace formats e.g.: iotwebconf::TextParameter
IotWebConfTextParameter mqttServerParam = IotWebConfTextParameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);

bool needMqttConnect = false;
bool needReset = false;
unsigned long lastMqttConnectionAttempt = 0;
int needAction = NO_ACTION;
int state = LOW;
unsigned long lastAction = 0;
char mqttActionTopic[STRING_LEN];
char mqttStatusTopic[STRING_LEN];

void setup() 
{
  Serial.begin(115200); // See "Note on relay pin"!
  Serial.println();
  Serial.println("Starting up...");

  pinMode(RELAY_PIN, OUTPUT);

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(BUTTON_PIN);
  iotWebConf.addSystemParameter(&mqttServerParam);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });

  // -- Initializing the configuration.
  bool validConfig = iotWebConf.init();
  if (!validConfig)
  {
    mqttServerValue[0] = '\0';
  }

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  // -- Prepare dynamic topic names
  String temp = String(MQTT_TOPIC_PREFIX);
  temp += iotWebConf.getThingName();
  temp += "/action";
  temp.toCharArray(mqttActionTopic, STRING_LEN);
  temp = String(MQTT_TOPIC_PREFIX);
  temp += iotWebConf.getThingName();
  temp += "/status";
  temp.toCharArray(mqttStatusTopic, STRING_LEN);

  mqttClient.begin(mqttServerValue, net);
  mqttClient.onMessage(mqttMessageReceived);
  
  Serial.println("Ready.");
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
  mqttClient.loop();
  
  if (needMqttConnect)
  {
    if (connectMqtt())
    {
      needMqttConnect = false;
    }
  }
  else if ((iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE) && (!mqttClient.connected()))
  {
    Serial.println("MQTT reconnect");
    connectMqtt();
  }

  if (needReset)
  {
    Serial.println("Rebooting after 1 second.");
    iotWebConf.delay(1000);
    ESP.restart();
  }

  unsigned long now = millis();

  // -- Check for button push
  if ((digitalRead(BUTTON_PIN) == LOW)
    && ( ACTION_FEQ_LIMIT < now - lastAction))
  {
    needAction = 1 - state; // -- Invert the state
  }
  
  if ((needAction != NO_ACTION)
    && ( ACTION_FEQ_LIMIT < now - lastAction))
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
    mqttClient.publish(mqttStatusTopic, state == HIGH ? "ON" : "OFF", true, 1);
    mqttClient.publish(mqttActionTopic, state == HIGH ? "ON" : "OFF", true, 1);
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
  String s = F("<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>");
  s += iotWebConf.getHtmlFormatProvider()->getStyle();
  s += "<title>IotWebConf 07 MQTT Relay</title></head><body>";
  s += iotWebConf.getThingName();
  s += "<div>State: ";
  s += (state == HIGH ? "ON" : "OFF");
  s += "</div>";
  s += "<button type='button' onclick=\"location.href='';\" >Refresh</button>";
  s += "<div>Go to <a href='config'>configure page</a> to change values.</div>";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void wifiConnected()
{
  needMqttConnect = true;
}

void configSaved()
{
  Serial.println("Configuration was updated.");
  needReset = true;
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
  Serial.println("Validating form.");
  bool valid = true;

  int l = webRequestWrapper->arg(mqttServerParam.getId()).length();
  if (l < 3)
  {
    mqttServerParam.errorMessage = "Please provide at least 3 characters!";
    valid = false;
  }

  return valid;
}

bool connectMqtt() {
  unsigned long now = millis();
  if (1000 > now - lastMqttConnectionAttempt)
  {
    // Do not repeat within 1 sec.
    return false;
  }
  Serial.println("Connecting to MQTT server...");
  if (!mqttClient.connect(iotWebConf.getThingName())) {
    lastMqttConnectionAttempt = now;
    return false;
  }
  Serial.println("Connected!");

  mqttClient.subscribe(mqttActionTopic);
  mqttClient.publish(mqttStatusTopic, state == HIGH ? "ON" : "OFF", true, 1);
  mqttClient.publish(mqttActionTopic, state == HIGH ? "ON" : "OFF", true, 1);

  return true;
}

void mqttMessageReceived(String &topic, String &payload)
{
  Serial.println("Incoming: " + topic + " - " + payload);
  
  if (topic.endsWith("action"))
  {
    needAction = payload.equals("ON") ? HIGH : LOW;
    if (needAction == state)
    {
      needAction = NO_ACTION;
    }
  }
}

