/***********************************************************************
  FossaGroundStation.ini - GroundStation firmware
  
  Copyright (C) 2020 @G4lile0, @gmag12 and @dev_4m1g0

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  ***********************************************************************

  The aim of this project is to create an open network of ground stations
  for the Fossa Satellites distributed all over the world and connected
  through Internet.
  This project is based on ESP32 boards and is compatible with sx126x and
  sx127x you can build you own board using one of these modules but most
  of us use a development board like the ones listed in the Supported
  boards section.
  The developers of this project have no relation with the Fossa team in
  charge of the mission, we are passionate about space and created this
  project to be able to trackand use the satellites as well as supporting
  the mission.

  Supported boards
    Heltec WiFi LoRa 32 V1 (433MHz SX1278)
    Heltec WiFi LoRa 32 V2 (433MHz SX1278)
    TTGO LoRa32 V1 (433MHz SX1278)
    TTGO LoRa32 V2 (433MHz SX1278)

  Supported modules
    sx126x
    sx127x

    World Map with active Ground Stations and satellite stimated possition 
    Main community chat: https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q

    In order to onfigure your Ground Station please open a private chat to get your credentials https://t.me/fossa_updates_bot
    Data channel (station status and received packets): https://t.me/FOSSASAT_DATA
    Test channel (simulator packets received by test groundstations): https://t.me/FOSSASAT_TEST

    Developers:
      @gmag12       https://twitter.com/gmag12
      @dev_4m1g0    https://twitter.com/dev_4m1g0
      @g4lile0      https://twitter.com/G4lile0

**************************************************************************/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "src/ConfigManager/ConfigManager.h"
#include "src/Comms/Comms.h"
#include "src/Display/Display.h"

#include "src/Mqtt/esp32_mqtt_client.h"
#include "ArduinoJson.h"
#include "src/Status.h"
#include "src/Radio/Radio.h"

#include "src/ArduinoOTA/ArduinoOTA.h"

ConfigManager configManager;
Esp32_mqtt_clientClass mqtt;
Radio radio(configManager);

const char* message[32];

void manageMQTTEvent (esp_mqtt_event_id_t event);
void manageMQTTData (char* topic, size_t topic_len, char* payload, size_t payload_len);
void manageSatPosOled(char* payload, size_t payload_len);

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0; // 3600;         // 3600 for Spain
const int   daylightOffset_sec = 0; // 3600;
void printLocalTime();

// Global status
Status status;

void printControls();

void welcome_message();
void json_system_info();
void json_message();
void json_pong();

void sendPing();
void switchTestmode();
void requestInfo();
void requestPacketInfo();
void requestRetransmit();

void wifiConnected() {
  configManager.printConfig();
  arduino_ota_setup();
  displayShowConnected();

  radio.init();
  if (!radio.isReady()){
    Serial.println("LoRa initialization failed. Please connect to " + WiFi.localIP().toString() + " and make sure the board selected matches your hardware.");
    displayShowLoRaError();
  }

  mqtt.init(configManager.getMqttServer(), configManager.getMqttPort(), configManager.getMqttUser(), configManager.getMqttPass());
  String topic = "fossa/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/status";
  mqtt.setLastWill(topic.c_str());
  mqtt.onEvent(manageMQTTEvent);
  mqtt.onReceive(manageMQTTData);
  mqtt.begin();

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (strcmp (configManager.getTZ(), "")) {
	  setenv("TZ", configManager.getTZ(), 1);
	  ESP_LOGD (LOG_TAG, "Set timezone value as %s", configManager.getTZ());
	  tzset();
  }

  printLocalTime();
  delay (1000); // wait to show the connected screen

  // TODO: Make this beautiful
  displayShowWaitingMqttConnection();
  Serial.println ("Waiting for MQTT connection. Connect to the config panel on the ip: " + WiFi.localIP().toString() + " to review the MQTT connection credentials.");
  int i = 0;
  while (!status.mqtt_connected) {
    if (i++ > 120) {// 1m
      Serial.println (" MQTT unable to connect after 30s, restarting...");
      ESP.restart();
    }
	  Serial.print ('.');
    // FIXME: This is a temporal hack! we should not block the main thread waiting for the mqtt connection
    // in the meantime we call the config loop to make it alive.
    configManager.delay(500);
  }
  Serial.println (" Connected !!!");
  
  printControls();
}

void setup() {
  Serial.begin(115200);
  delay(299);
  Serial.printf("Fossa Ground station Version %d\n", status.version);
  configManager.setWifiConnectionCallback(wifiConnected);
  configManager.init();
  // make sure to call doLoop at least once before starting to use the configManager
  configManager.doLoop();
  pinMode (configManager.getBoardConfig().PROG__BUTTON, INPUT_PULLUP);
  displayInit();
  displayShowInitialCredits();

#define WAIT_FOR_BUTTON 3000
#define RESET_BUTTON_TIME 5000
  unsigned long start_waiting_for_button = millis ();
  unsigned long button_pushed_at;
  ESP_LOGI (LOG_TAG, "Waiting for reset config button");
  bool button_pushed = false;
  while (millis () - start_waiting_for_button < WAIT_FOR_BUTTON) {

	  if (!digitalRead (configManager.getBoardConfig().PROG__BUTTON)) {
		  button_pushed = true;
		  button_pushed_at = millis ();
		  ESP_LOGI (LOG_TAG, "Reset button pushed");
		  while (millis () - button_pushed_at < RESET_BUTTON_TIME) {
			  if (digitalRead (configManager.getBoardConfig().PROG__BUTTON)) {
				  ESP_LOGI (LOG_TAG, "Reset button released");
				  button_pushed = false;
				  break;
			  }
		  }
		  if (button_pushed) {
			  ESP_LOGI (LOG_TAG, "Reset config triggered");
			  WiFi.begin ("0", "0");
			  WiFi.disconnect ();
		  }
	  }
  }

  if (button_pushed) {
    configManager.resetAPConfig();
    ESP.restart();
  }
  
  if (configManager.isApMode()) {
    displayShowApMode();
  } 
  else {
    displayShowStaMode();
  }
  delay(500);  
}

void loop() {
  configManager.doLoop();

  static bool wasConnected = false;
  if (!configManager.isConnected()){
    if (wasConnected){
      if (configManager.isApMode())
        displayShowApMode();
      else 
        displayShowStaMode();
    }
    return;
  }
  wasConnected = true;

  if (!radio.isReady()) {
    return;
  }

  displayUpdate();
  
  if(Serial.available()) {
    radio.disableInterrupt();

    // get the first character
    char serialCmd = Serial.read();

    // wait for a bit to receive any trailing characters
    delay(50);

    // dump the serial buffer
    while(Serial.available()) {
      Serial.read();
    }

    // process serial command
    switch(serialCmd) {
      case 'p':
        radio.sendPing();
        break;
      case 'i':
        radio.requestInfo();
        break;
      case 'l':
        radio.requestPacketInfo();
        break;
      case 'r':
        Serial.println(F("Enter message to be sent:"));
        Serial.println(F("(max 32 characters, end with LF or CR+LF)"));
        {
          // get data to be retransmited
          char optData[32];
          uint8_t bufferPos = 0;
          while(bufferPos < 32) {
            while(!Serial.available());
            char c = Serial.read();
            Serial.print(c);
            if((c != '\r') && (c != '\n')) {
              optData[bufferPos] = c;
              bufferPos++;
            } else {
              break;
            }
          }
          optData[bufferPos] = '\0';

          // wait for a bit to receive any trailing characters
          delay(100);

          // dump the serial buffer
          while(Serial.available()) {
            Serial.read();
          }

          Serial.println();
          radio.requestRetransmit(optData);
        }
        break;
      case 'e':
        configManager.resetAllConfig();
        ESP.restart();
        break;
      case 't':
        switchTestmode();
        ESP.restart();
        break;
      case 'b':
        ESP.restart();
        break;
       
      default:
        Serial.print(F("Unknown command: "));
        Serial.println(serialCmd);
        break;
    }

    radio.enableInterrupt();
  }

  uint8_t *respOptData; // Warning: this needs to be freed
  size_t respLen = 0;
  uint8_t functionId = 0;
  uint8_t error = radio.listen(respOptData, respLen, functionId);
  if (!error)
    processReceivedFrame(functionId, respOptData, respLen);

  delete[] respOptData;
  respOptData = nullptr;
  
  static unsigned long last_connection_fail = millis();
  if (!status.mqtt_connected){
    if (millis() - last_connection_fail > 300000){ // 5m
      Serial.println("MQTT Disconnected, restarting...");
      ESP.restart();
    }
  } else {
    last_connection_fail = millis();
  }

  ArduinoOTA.handle ();
}

void processReceivedFrame(uint8_t functionId, uint8_t *respOptData, size_t respLen) {
  switch(functionId) {
    case RESP_PONG:
      Serial.println(F("Pong!"));
      json_pong();
      break;

    case RESP_SYSTEM_INFO:
      Serial.println(F("System info:"));

      Serial.print(F("batteryChargingVoltage = "));
      status.sysInfo.batteryChargingVoltage = FCP_Get_Battery_Charging_Voltage(respOptData);
      Serial.println(FCP_Get_Battery_Charging_Voltage(respOptData));
      
      Serial.print(F("batteryChargingCurrent = "));
      status.sysInfo.batteryChargingCurrent = (FCP_Get_Battery_Charging_Current(respOptData), 4);
      Serial.println(FCP_Get_Battery_Charging_Current(respOptData), 4);

      Serial.print(F("batteryVoltage = "));
      status.sysInfo.batteryVoltage=FCP_Get_Battery_Voltage(respOptData);
      Serial.println(FCP_Get_Battery_Voltage(respOptData));          

      Serial.print(F("solarCellAVoltage = "));
      status.sysInfo.solarCellAVoltage= FCP_Get_Solar_Cell_Voltage(0, respOptData);
      Serial.println(FCP_Get_Solar_Cell_Voltage(0, respOptData));

      Serial.print(F("solarCellBVoltage = "));
      status.sysInfo.solarCellBVoltage= FCP_Get_Solar_Cell_Voltage(1, respOptData);
      Serial.println(FCP_Get_Solar_Cell_Voltage(1, respOptData));

      Serial.print(F("solarCellCVoltage = "));
      status.sysInfo.solarCellCVoltage= FCP_Get_Solar_Cell_Voltage(2, respOptData);
      Serial.println(FCP_Get_Solar_Cell_Voltage(2, respOptData));

      Serial.print(F("batteryTemperature = "));
      status.sysInfo.batteryTemperature=FCP_Get_Battery_Temperature(respOptData);
      Serial.println(FCP_Get_Battery_Temperature(respOptData));

      Serial.print(F("boardTemperature = "));
      status.sysInfo.boardTemperature=FCP_Get_Board_Temperature(respOptData);
      Serial.println(FCP_Get_Board_Temperature(respOptData));

      Serial.print(F("mcuTemperature = "));
      status.sysInfo.mcuTemperature =FCP_Get_MCU_Temperature(respOptData);
      Serial.println(FCP_Get_MCU_Temperature(respOptData));

      Serial.print(F("resetCounter = "));
      status.sysInfo.resetCounter=FCP_Get_Reset_Counter(respOptData);
      Serial.println(FCP_Get_Reset_Counter(respOptData));

      Serial.print(F("powerConfig = 0b"));
      status.sysInfo.powerConfig=FCP_Get_Power_Configuration(respOptData);
      Serial.println(FCP_Get_Power_Configuration(respOptData), BIN);

      json_system_info();
      break;

    case RESP_LAST_PACKET_INFO:
      Serial.println(F("Last packet info:"));

      Serial.print(F("SNR = "));
      Serial.print(respOptData[0] / 4.0);
      Serial.println(F(" dB"));

      Serial.print(F("RSSI = "));
      Serial.print(respOptData[1] / -2.0);
      Serial.println(F(" dBm"));
      break;

    case RESP_REPEATED_MESSAGE:
      Serial.println(F("Got repeated message:"));
      Serial.println((char*)respOptData);
      json_message((char*)respOptData,respLen);
      break;

    default:
      Serial.println(F("Unknown function ID!"));
      break;
  }
}


void  welcome_message (void) {
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(16);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();  // G4lile0
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["version"] = status.version;

  serializeJson(doc, Serial);
  String topic = "fossa/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/welcome";
  char buffer[512];
  size_t n = serializeJson(doc, buffer);
  mqtt.publish(topic.c_str(), buffer,n );
  ESP_LOGI (LOG_TAG, "Wellcome sent");
}

void  json_system_info(void) {
  time_t now;
  time(&now);
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(19);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();  // G4lile0
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["unix_GS_time"] = now;
  doc["batteryChargingVoltage"] = status.sysInfo.batteryChargingVoltage;
  doc["batteryChargingCurrent"] = status.sysInfo.batteryChargingCurrent;
  doc["batteryVoltage"] = status.sysInfo.batteryVoltage;
  doc["solarCellAVoltage"] = status.sysInfo.solarCellAVoltage;
  doc["solarCellBVoltage"] = status.sysInfo.solarCellBVoltage;
  doc["solarCellCVoltage"] = status.sysInfo.solarCellCVoltage;
  doc["batteryTemperature"] = status.sysInfo.batteryTemperature;
  doc["boardTemperature"] = status.sysInfo.boardTemperature;
  doc["mcuTemperature"] = status.sysInfo.mcuTemperature;
  doc["resetCounter"] = status.sysInfo.resetCounter;
  doc["powerConfig"] = status.sysInfo.powerConfig;
  serializeJson(doc, Serial);
  String topic = "fossa/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/sys_info";
  char buffer[512];
  serializeJson(doc, buffer);
  size_t n = serializeJson(doc, buffer);
  mqtt.publish(topic.c_str(), buffer,n );
}


void  json_message(char* frame, size_t respLen) {
  time_t now;
  time(&now);
  Serial.println(String(respLen));
  char tmp[respLen+1];
  memcpy(tmp, frame, respLen);
  tmp[respLen-12] = '\0';

      // if special miniTTN message   
  Serial.println(String(frame[0]));
  Serial.println(String(frame[1]));
  Serial.println(String(frame[2]));
//          if ((frame[0]=='0x54') &&  (frame[1]=='0x30') && (frame[2]=='0x40'))
  if ((frame[0]=='T') &&  (frame[1]=='0') && (frame[2]=='@'))
  {
    Serial.println("mensaje miniTTN");
    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(11) +JSON_ARRAY_SIZE(respLen-12);
    DynamicJsonDocument doc(capacity);
    doc["station"] = configManager.getThingName();  // G4lile0
    JsonArray station_location = doc.createNestedArray("station_location");
    station_location.add(configManager.getLongitude());
    station_location.add(configManager.getLongitude());
    doc["rssi"] = status.lastPacketInfo.rssi;
    doc["snr"] = status.lastPacketInfo.snr;
    doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
    doc["unix_GS_time"] = now;
    JsonArray msgTTN = doc.createNestedArray("msgTTN");

    for (byte i=0 ; i<  (respLen-12);i++) {
      msgTTN.add(String(tmp[i], HEX));
    }
    serializeJson(doc, Serial);
    String topic = "fossa/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/miniTTN";

    char buffer[256];
    serializeJson(doc, buffer);
    size_t n = serializeJson(doc, buffer);
    mqtt.publish(topic.c_str(), buffer,n );
  }
  else {
    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(11);
    DynamicJsonDocument doc(capacity);
    doc["station"] = configManager.getThingName();  // G4lile0
    JsonArray station_location = doc.createNestedArray("station_location");
    station_location.add(configManager.getLatitude());
    station_location.add(configManager.getLongitude());
    doc["rssi"] = status.lastPacketInfo.rssi;
    doc["snr"] = status.lastPacketInfo.snr;
    doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
    doc["unix_GS_time"] = now;
  //          doc["len"] = respLen;
    doc["msg"] = String(tmp);
  //          doc["msg"] = String(frame);

    serializeJson(doc, Serial);
    String topic = "fossa/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/msg";
    
    char buffer[256];
    serializeJson(doc, buffer);
    size_t n = serializeJson(doc, buffer);
    mqtt.publish(topic.c_str(), buffer,n );
  }
}

void  json_pong(void) {
  //// JSON
  time_t now;
  time(&now);
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(7);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();  // G4lile0
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["unix_GS_time"] = now;
  doc["pong"] = 1;
  serializeJson(doc, Serial);
  String topic = "fossa/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/pong";
  char buffer[256];
  serializeJson(doc, buffer);
  size_t n = serializeJson(doc, buffer);
  mqtt.publish(topic.c_str(), buffer,n );
}











void  switchTestmode() {
  char temp_station[32];
  if ((configManager.getThingName()[0]=='t') &&  (configManager.getThingName()[1]=='e') && (configManager.getThingName()[2]=='s') && (configManager.getThingName()[4]=='_')) {
    Serial.println(F("Changed from test mode to normal mode"));
    for (byte a=5; a<=strlen(configManager.getThingName()); a++ ) {
      configManager.getThingName()[a-5]=configManager.getThingName()[a];
    }
  }
  else
  {
    strcpy(temp_station,"test_");
    strcat(temp_station,configManager.getThingName());
    strcpy(configManager.getThingName(),temp_station);
    Serial.println(F("Changed from normal mode to test mode"));
  }

  configManager.configSave();
}

void manageMQTTEvent (esp_mqtt_event_id_t event) {
  if (event == MQTT_EVENT_CONNECTED) {
	  status.mqtt_connected = true;
    String topic = "fossa/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/data/#";
    mqtt.subscribe (topic.c_str());
    mqtt.subscribe ("fossa/global/sat_pos_oled");
	  welcome_message ();
    
  } else   if (event == MQTT_EVENT_DISCONNECTED) {
    status.mqtt_connected = false;
  }
}
void manageSatPosOled(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  status.satPos[0] = doc[0];
  status.satPos[1] = doc[1];
}

void manageMQTTData (char* topic, size_t topic_len, char* payload, size_t payload_len) {
  // Don't use Serial.print here. It will not work. Use ESP_LOG or printf instead.
  ESP_LOGI (LOG_TAG,"Received MQTT message: %.*s : %.*s\n", topic_len, topic, payload_len, payload);
  char topicStr[topic_len+1];
  memcpy(topicStr, topic, topic_len);
  topicStr[topic_len] = '\0';
  if (!strcmp(topicStr, "fossa/global/sat_pos_oled")) {
    manageSatPosOled(payload, payload_len);
  }
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// function to print controls
void printControls() {
  Serial.println(F("------------- Controls -------------"));
  Serial.println(F("p - send ping frame"));
  Serial.println(F("i - request satellite info"));
  Serial.println(F("l - request last packet info"));
  Serial.println(F("r - send message to be retransmitted"));
  Serial.println(F("t - change the test mode and restart"));
  Serial.println(F("e - erase board config and reset"));
  Serial.println(F("b - reboot the board"));
  Serial.println(F("------------------------------------"));
}