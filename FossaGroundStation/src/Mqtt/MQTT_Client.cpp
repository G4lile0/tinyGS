/*
  MQTTClient.cpp - MQTT connection class
  
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
*/

#include "MQTT_Client.h"
#include "ArduinoJson.h"
#include "../Radio/Radio.h"

MQTT_Client::MQTT_Client() 
: PubSubClient(espClient)
{ }

void MQTT_Client::loop() {
  if (!connected() && millis() - lastConnectionAtempt > reconnectionInterval) {
    lastConnectionAtempt = millis();
    connectionAtempts++;
    status.mqtt_connected = false;
    reconnect();
  }
  else {
    connectionAtempts = 0;
    status.mqtt_connected = true;
  }

  if (connectionAtempts > connectionTimeout) {
    Serial.println("Unable to connect to MQTT Server after many atempts. Restarting...");
    ESP.restart();
  }

  PubSubClient::loop();

  unsigned long now = millis();
  if (now - lastPing > pingInterval) {
    lastPing = now;
    publish(buildTopic(topicPing).c_str(), "1");
  }
}

void MQTT_Client::reconnect() {
  ConfigManager& configManager = ConfigManager::getInstance();
  uint64_t chipId = ESP.getEfuseMac();
  char clientId[13];
  sprintf(clientId, "%04X%08X",(uint16_t)(chipId>>32), (uint32_t)chipId);

  Serial.print("Attempting MQTT connection...");
  Serial.println ("If this is taking more than expected, connect to the config panel on the ip: " + WiFi.localIP().toString() + " to review the MQTT connection credentials.");
  if (connect(clientId, configManager.getMqttUser(), configManager.getMqttPass(), buildTopic(topicStatus).c_str(), 2, false, "0")) {
    Serial.println("connected");
    subscribeToAll();
    sendWelcome();
  }
  else {
    Serial.print("failed, rc=");
    Serial.print(state());
  }
}

String MQTT_Client::buildTopic(const char* topic){
  ConfigManager& configManager = ConfigManager::getInstance();
  return String(topicStart) + "/" + String(configManager.getMqttUser()) + "/" + String(configManager.getThingName()) + "/" +  String(topic);
}

void MQTT_Client::subscribeToAll() {
  String sat_pos_oled = String(topicStart) + "/global/#";
  subscribe(buildTopic(topicData).c_str());
  subscribe(sat_pos_oled.c_str());
}

void MQTT_Client::sendWelcome() {
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);
  status.tx = configManager.getTx();
  status.remoteTune = configManager.getRemoteTune();
  status.telemetry3rd = configManager.getTelemetry3rd();
  status.test = configManager.getTest(); 
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(16);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["version"] = status.version;
  doc["board"] = configManager.getBoard();
  doc["tx"] = status.tx;
  doc["remoteTune"] = status.remoteTune;
  doc["telemetry3d"] = status.telemetry3rd;
  doc["test"] = status.test;
  doc["unix_GS_time"] = now;
  serializeJson(doc, Serial);
  char buffer[512];
  publish(buildTopic(topicWelcome).c_str(), buffer,false);
}

void  MQTT_Client::sendSystemInfo() {
  ConfigManager& configManager = ConfigManager::getInstance();
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
  char buffer[512];
  serializeJson(doc, buffer);
  publish(buildTopic(topicSysInfo).c_str(), buffer, false);
}

void  MQTT_Client::sendPong() {
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(7);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["unix_GS_time"] = now;
  doc["pong"] = 1;
  serializeJson(doc, Serial);
  char buffer[256];
  serializeJson(doc, buffer);
  publish(buildTopic(topicPong).c_str(), buffer, false);
}

void  MQTT_Client::sendMessage(char* frame, size_t respLen) {
  ConfigManager& configManager = ConfigManager::getInstance();
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
  // if ((frame[0]=='0x54') &&  (frame[1]=='0x30') && (frame[2]=='0x40'))
  if ((frame[0]=='T') &&  (frame[1]=='0') && (frame[2]=='@'))
  {
    Serial.println("mensaje miniTTN");
    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(11) + JSON_ARRAY_SIZE(respLen-12);
    DynamicJsonDocument doc(capacity);
    doc["station"] = configManager.getThingName();
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
    char buffer[256];
    serializeJson(doc, buffer);
    publish(buildTopic(topicMiniTTN).c_str(), buffer, false);
  }
  else {
    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(11);
    DynamicJsonDocument doc(capacity);
    doc["station"] = configManager.getThingName();
    JsonArray station_location = doc.createNestedArray("station_location");
    station_location.add(configManager.getLatitude());
    station_location.add(configManager.getLongitude());
    doc["rssi"] = status.lastPacketInfo.rssi;
    doc["snr"] = status.lastPacketInfo.snr;
    doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
    doc["unix_GS_time"] = now;
    doc["msg"] = String(tmp);

    serializeJson(doc, Serial);
    char buffer[256];
    serializeJson(doc, buffer);
    publish(buildTopic(topicMsg).c_str(), buffer, false);
  }
}

void  MQTT_Client::sendRawPacket(String packet) {
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(18);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["mode"] = status.modeminfo.modem_mode;
  doc["frequency"] = status.modeminfo.frequency;
  doc["satellite"] = status.modeminfo.satellite;
 
  if (String(status.modeminfo.modem_mode)=="LoRa") {
      doc["sf"] = status.modeminfo.sf;
      doc["cr"] = status.modeminfo.cr;
      doc["bw"] = status.modeminfo.bw;
  
  } else {
      doc["bitrate"] = status.modeminfo.bitrate;
      doc["freqdev"] = status.modeminfo.freqDev;
      doc["rxBw"] = status.modeminfo.rxBw;
    }
  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["unix_GS_time"] = now;
  doc["CRC_error"] = status.lastPacketInfo.crc_error;
  doc["data"] = packet.c_str();
  
  serializeJson(doc, Serial);
  char buffer[1024];
  serializeJson(doc, buffer);
  publish(buildTopic(topicRawPacket).c_str(), buffer, false);
}



void  MQTT_Client::sendStatus() {
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(20);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["mode"] = status.modeminfo.modem_mode;
  doc["frequency"] = status.modeminfo.frequency;
  doc["satellite"] = status.modeminfo.satellite;
 
  if (String(status.modeminfo.modem_mode)=="LoRa") {
      doc["sf"] = status.modeminfo.sf;
      doc["cr"] = status.modeminfo.cr;
      doc["bw"] = status.modeminfo.bw;

  } else {
      doc["bitrate"] = status.modeminfo.bitrate;
      doc["freqdev"] = status.modeminfo.freqDev;
      doc["rxBw"] = status.modeminfo.rxBw;
    }

  doc["pl"] = status.modeminfo.preambleLength;
  doc["CRC"] = status.modeminfo.crc;
  doc["FLDRO"] = status.modeminfo.fldro;
  doc["OOK"] = status.modeminfo.enableOOK;
  doc["Shaping"] = status.modeminfo.dataShaping;

  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["unix_GS_time"] = now;
  doc["CRC_error"] = status.lastPacketInfo.crc_error;
  //doc["data"] = packet.c_str();
  
  serializeJson(doc, Serial);
  char buffer[1024];
  serializeJson(doc, buffer);
  publish(buildTopic(topicSendStatus).c_str(), buffer, false);
}







void MQTT_Client::manageMQTTData(char *topic, uint8_t *payload, unsigned int length) {
  Radio& radio = Radio::getInstance();
  if (!strcmp(topic, "fossa/global/sat_pos_oled")) {
    manageSatPosOled((char*)payload, length);
  }

// Remote_Frame_Local_A          -m "[]" -t fossa/global/global_frame
//
// [number of strings,
// [font,TextAlignment,x,y,"string text"],
// ...
// ]
//
if (!strcmp(topic, "fossa/global/global_frame")) {    
//if (!strcmp(topic, "fossa/g4lile0/test_G4lile0_new/data/remote/global_frame")) {    
    radio.remote_global_frame((char*)payload, length);
  }

 // Remote_Reset        -m "[1]" -t fossa/g4lile0/test_G4lile0_new/remote/reset
 if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteReset)).c_str()).c_str())) {
    ESP.restart();
  }

 // Remote_switchTestmode(       -m "[1]" -t fossa/g4lile0/test_G4lile0_new/remote/test
 if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteTest)).c_str()).c_str())) {
   ConfigManager& configManager = ConfigManager::getInstance();

  if (configManager.getTest()){
      // TODO getTest to false
      Serial.println(F("Changed from test mode to normal mode"));
  } else {
      // TODO getTest to true
      Serial.println(F("Changed from normal mode to test mode"));
  }

  configManager.configSave();
  status.test = configManager.getTest();
  // reset .. o welcome?


   /*
    TODO borrar
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
  ESP.restart();
*/

    }



// Remote_Ping           -m "[1]" -t fossa/g4lile0/test_G4lile0_new/remote/ping
 if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemotePing)).c_str()).c_str())) {
    radio.sendPing();
  }

// Remote_Frequency       -m "[434.8]" -t fossa/g4lile0/test_G4lile0_new/data/remote/freq
 if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteFreq)).c_str()).c_str()) || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteFreq) ).c_str()) && status.remoteTune ))  {
    radio.remote_freq((char*)payload, length);
  }
// Remote_Lora_Bandwidth  -m "[250]" -t fossa/g4lile0/test_G4lile0_new/data/remote/bw
 if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteBw)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteBw) ).c_str())&& status.remoteTune )) {
    radio.remote_bw((char*)payload, length);
  }
// Remote_spreading factor -m "[11]" -t fossa/g4lile0/test_G4lile0_new/data/remote/sf 
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteSf)).c_str()).c_str())   || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteSf) ).c_str()) && status.remoteTune )) {
    radio.remote_sf((char*)payload, length);
  }
// Remote_Coding rate       -m "[8]" -t fossa/g4lile0/test_G4lile0_new/data/remote/cr
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteCr)).c_str()).c_str())   || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteCr) ).c_str()) && status.remoteTune )) {
    radio.remote_cr((char*)payload, length);
  }
// Remote_Crc               -m "[0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/crc
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteCrc)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteCrc) ).c_str()) && status.remoteTune )) {
    radio.remote_crc((char*)payload, length);
  }

// Remote_LoRa_syncword          -m "[8,1,2,3,4,5,6,7,8,9]" -t fossa/g4lile0/test_G4lile0_new/data/remote/lsw
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteLsw)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteLsw) ).c_str()) && status.remoteTune )) {
    radio.remote_lsw((char*)payload, length);
  }

// Remote_Force LDRO        -m "[0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fldro
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteFldro)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteFldro) ).c_str()) && status.remoteTune )) {
    radio.remote_fldro((char*)payload, length);
  }

// Remote_auto LDRO        -m "[0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/aldro
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteAldro)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteAldro) ).c_str()) && status.remoteTune )) {
    radio.remote_aldro((char*)payload, length);
  }


// Remote_Preamble Lenght   -m "[8]" -t fossa/g4lile0/test_G4lile0_new/data/remote/pl
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemotePl)).c_str()).c_str())   || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemotePl) ).c_str()) && status.remoteTune )) {
    radio.remote_pl((char*)payload, length);
  }

// Remote_Begin_Lora       -m "[437.7,125.0,11,8,18,11,120,8,0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/begin_lora
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteBl)).c_str()).c_str())   || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteBl) ).c_str()) && status.remoteTune )) {
    radio.remote_begin_lora((char*)payload, length);
  }
// Remote_Begin_FSK       -m "[433.5,100.0,10.0,250.0,10,100,16,0,0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/begin_fsk
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteFs)).c_str()).c_str())   || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteFs) ).c_str()) && status.remoteTune )) {
    radio.remote_begin_fsk((char*)payload, length);
  }

// Remote_FSK_BitRate      -m "[250]" -t fossa/g4lile0/test_G4lile0_new/data/remote/br
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteBr)).c_str()).c_str())   || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteBr) ).c_str()) && status.remoteTune )) {
    radio.remote_br((char*)payload, length);
  }

// Remote_FSK_FrequencyDeviation  -m "[10.0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/Fd
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteFd)).c_str()).c_str())   || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteFd) ).c_str()) && status.remoteTune )) {
    radio.remote_fd((char*)payload, length);
  }

// Remote_FSK_RxBandwidth       -m "[125]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fbw
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteFbw)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteFbw) ).c_str()) && status.remoteTune )) {
    radio.remote_fbw((char*)payload, length);
  }

// Remote_FSK_syncword          -m "[8,1,2,3,4,5,6,7,8,9]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fsw
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteFsw)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteFsw) ).c_str()) && status.remoteTune )) {
    radio.remote_fsw((char*)payload, length);
  }

// Remote_FSK_Set_OOK + DataShapingOOK(only sx1278)     -m "[1,2]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fok
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteFook)).c_str()).c_str()) || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteFook) ).c_str()) && status.remoteTune )) {
    radio.remote_fook((char*)payload, length);
  }


// Remote_Satellite_Name       -m "[\"FossaSat-3\"]" -t fossa/g4lile0/test_G4lile0_new/data/remote/sat
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteSat)).c_str()).c_str())  || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicRemoteSat) ).c_str()) && status.remoteTune )) {
    radio.remote_sat((char*)payload, length);
  }

// Remote_Frame_Local_       -m "[\"FossaSat-3\"]" -t fossa/g4lile0/test_G4lile0_new/data/remote/sat
if ((!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteLocalFrame)).c_str()).c_str()) && status.remoteTune )) {
    radio.remote_local_frame((char*)payload, length);
  }

// Remote_Status       -m "[1]"     -t fossa/g4lile0/test_G4lile0_new/data/remote/status
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicRemoteStatus)).c_str()).c_str())) {
    radio.remote_status((char*)payload, length);
  }

 // GOD MODE  With great power comes great responsibility!
// SPIsetRegValue  (only sx1278)     -m "[1,2,3,4,5]" -t fossa/g4lile0/test_G4lile0_new/data/remote/SPIsetRegValue
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicSPIsetRegValue)).c_str()).c_str()) || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicSPIsetRegValue) ).c_str()) && status.remoteTune )) {
    radio.remote_SPIsetRegValue((char*)payload, length);
  }
// SPIwriteRegister  (only sx1278)     -m "[1,2]" -t fossa/g4lile0/test_G4lile0_new/data/remote/SPIwriteRegister
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicSPIwriteRegister)).c_str()).c_str()) || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicSPIwriteRegister) ).c_str()) && status.remoteTune )) {
    radio.remote_SPIwriteRegister((char*)payload, length);
  }

// SPIreadRegister            -m "[1]" -t fossa/g4lile0/test_G4lile0_new/data/remote/SPIreadRegister
if (!strcmp(topic, buildTopic((String(topicRemote) + String(topicSPIreadRegister)).c_str()).c_str()) || (!strcmp(topic, (String(topicGlobalRemote)+ String(topicSPIreadRegister) ).c_str()) && status.remoteTune )) {
    radio.remote_SPIreadRegister((char*)payload, length);
  }



// END GOD MODE

}


void MQTT_Client::manageSatPosOled(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  status.satPos[0] = doc[0];
  status.satPos[1] = doc[1];
}

// Helper class to use as a callback
void manageMQTTDataCallback(char *topic, uint8_t *payload, unsigned int length) {
  ESP_LOGI (LOG_TAG,"Received MQTT message: %s : %.*s\n", topic, length, payload);
  MQTT_Client::getInstance().manageMQTTData(topic, payload, length);
}

void MQTT_Client::begin() {
  ConfigManager& configManager = ConfigManager::getInstance();
  setServer(configManager.getMqttServer(), configManager.getMqttPort());
  setCallback(manageMQTTDataCallback);
}