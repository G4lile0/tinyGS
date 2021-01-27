/*
  MQTTClient.cpp - MQTT connection class
  
  Copyright (C) 2020 -2021 @G4lile0, @gmag12 and @dev_4m1g0

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
#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"
#include "../Radio/Radio.h"

MQTT_Client::MQTT_Client() 
: PubSubClient(espClient)
{ }

void MQTT_Client::loop() {
  if (!connected() && millis() - lastConnectionAtempt > reconnectionInterval)
  {
    lastConnectionAtempt = millis();
    connectionAtempts++;
    status.mqtt_connected = false;
    reconnect();
  }
  else
  {
    connectionAtempts = 0;
    status.mqtt_connected = true;
  }

  if (connectionAtempts > connectionTimeout)
  {
    Serial.println("Unable to connect to MQTT Server after many atempts. Restarting...");
    ESP.restart();
  }

  PubSubClient::loop();

  unsigned long now = millis();
  if (now - lastPing > pingInterval)
  {
    lastPing = now;
    publish(buildTopic(teleTopic, topicPing).c_str(), "1");
  }
}

void MQTT_Client::reconnect()
{
  ConfigManager& configManager = ConfigManager::getInstance();
  uint64_t chipId = ESP.getEfuseMac();
  char clientId[13];
  sprintf(clientId, "%04X%08X",(uint16_t)(chipId>>32), (uint32_t)chipId);

  Serial.print("Attempting MQTT connection...");
  Serial.println ("If this is taking more than expected, connect to the config panel on the ip: " + WiFi.localIP().toString() + " to review the MQTT connection credentials.");
  if (connect(clientId, configManager.getMqttUser(), configManager.getMqttPass(), buildTopic(teleTopic, topicStatus).c_str(), 2, false, "0")) {
    Serial.println("connected");
    subscribeToAll();
    sendWelcome();
  }
  else {
    Serial.print("failed, rc=");
    Serial.print(state());
  }
}

String MQTT_Client::buildTopic(const char* baseTopic, const char* cmnd)
{
  ConfigManager& configManager = ConfigManager::getInstance();
  String topic = baseTopic;
  topic.replace("%user%", configManager.getMqttUser());
  topic.replace("%station%", configManager.getThingName());
  topic.replace("%cmnd%", cmnd);

  return topic;
}

void MQTT_Client::subscribeToAll() {
  subscribe(buildTopic(globalTopic, "#").c_str());
  subscribe(buildTopic(cmndTopic, "#").c_str());
}

void MQTT_Client::sendWelcome()
{
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);

  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(11) + 22 + 20;
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["version"] = status.version;
  doc["git_version"] = status.git_version;
  doc["board"] = configManager.getBoard();
  doc["tx"] = configManager.getAllowTx();
  doc["remoteTune"] = configManager.getRemoteTune();
  doc["telemetry3d"] = configManager.getTelemetry3rd();
  doc["test"] = configManager.getTestMode();
  doc["unix_GS_time"] = now;
  doc["autoUpdate"] = configManager.getAutoUpdate();
  serializeJson(doc, Serial);
  char buffer[512];
  serializeJson(doc, buffer);
  publish(buildTopic(teleTopic, topicWelcome).c_str(), buffer, false);
}

void  MQTT_Client::sendRx(String packet)
{
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);
  struct timeval tv;
  gettimeofday(&tv, NULL);

  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(22);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["mode"] = status.modeminfo.modem_mode;
  doc["frequency"] = status.modeminfo.frequency;
  doc["satellite"] = status.modeminfo.satellite;
 
  if (String(status.modeminfo.modem_mode)=="LoRa")
  {
    doc["sf"] = status.modeminfo.sf;
    doc["cr"] = status.modeminfo.cr;
    doc["bw"] = status.modeminfo.bw;
  }
  else
  {
    doc["bitrate"] = status.modeminfo.bitrate;
    doc["freqdev"] = status.modeminfo.freqDev;
    doc["rxBw"] = status.modeminfo.rxBw;
  }

  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["unix_GS_time"] = now;
  doc["usec_time"] = (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;
  doc["time_offset"] = status.time_offset;
  doc["CRC_error"] = status.lastPacketInfo.crc_error;
  doc["data"] = packet.c_str();
  doc["NORAD"] = status.modeminfo.NORAD;
  doc["test"] = configManager.getTestMode();

  serializeJson(doc, Serial);
  char buffer[1536];
  serializeJson(doc, buffer);
  publish(buildTopic(teleTopic, topicRx).c_str(), buffer, false);
}

void  MQTT_Client::sendStatus()
{
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(28);
  DynamicJsonDocument doc(capacity);
  doc["station"] = configManager.getThingName();
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());

  doc["version"] = status.version;
  doc["board"] = configManager.getBoard();
  doc["tx"] = configManager.getAllowTx();
  doc["remoteTune"] = configManager.getRemoteTune();
  doc["telemetry3d"] = configManager.getTelemetry3rd();
  doc["test"] = configManager.getTestMode();

  doc["mode"] = status.modeminfo.modem_mode;
  doc["frequency"] = status.modeminfo.frequency;
  doc["satellite"] = status.modeminfo.satellite;
  doc["NORAD"] = status.modeminfo.NORAD;
 
  if (String(status.modeminfo.modem_mode)=="LoRa")
  {
    doc["sf"] = status.modeminfo.sf;
    doc["cr"] = status.modeminfo.cr;
    doc["bw"] = status.modeminfo.bw;
  }
  else
  {
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
  doc["CRC_error"] = status.lastPacketInfo.crc_error;
  doc["unix_GS_time"] = now;
  doc["usec_time"] = (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;
  doc["time_offset"] = status.time_offset;
    
  serializeJson(doc, Serial);
  char buffer[1024];
  serializeJson(doc, buffer);
  publish(buildTopic(statTopic, topicStatus).c_str(), buffer, false);
}

void MQTT_Client::manageMQTTData(char *topic, uint8_t *payload, unsigned int length)
{
  Radio& radio = Radio::getInstance();

  bool global = true;
  char* command;
  strtok(topic, "/");
  strtok(NULL, "/"); // tinygs
  if (strcmp(strtok(NULL, "/"), "global")) // user
  {
    global = false;
    strtok(NULL, "/"); // station
  }

  command = strtok(NULL, "/");

  if (!strcmp(command, commandSatPos)) {
    manageSatPosOled((char*)payload, length);
  }

  // Remote_Reset        -m "[1]" -t fossa/g4lile0/test_G4lile0_new/remote/reset
  if (!strcmp(command, commandReset))
  {
    ESP.restart();
  }

  // Remote_setTestmode(       -m "[1]" -t fossa/g4lile0/test_G4lile0_new/remote/test
  if (!strcmp(command, commandTest))
  {
    if (length < 1) return;
    ConfigManager& configManager = ConfigManager::getInstance();
    bool test = payload[0] - '0';
    Serial.print(F("Set Test Mode to ")); if (test) Serial.println(F("ON")); else Serial.println(F("OFF"));
    configManager.setTestMode(test);
  }

  // Remote_topicRemoteremoteTune       -m "[1]" -t fossa/g4lile0/test_G4lile0_new/remote/remoteTune
  if (!strcmp(command, commandRemoteTune))
  {
    if (length < 1) return;
    ConfigManager& configManager = ConfigManager::getInstance();
    bool tune = payload[0] - '0';
    Serial.println("");
    Serial.print(F("Set Remote Tune to ")); if (tune) Serial.println(F("ON")); else Serial.println(F("OFF"));
    configManager.setRemoteTune(tune);
  }

  // Remote_topicRemotetelemetry3rd      -m "[1]" -t fossa/g4lile0/test_G4lile0_new/remote/telemetry3rd
  if (!strcmp(command, commandRemoteTune))
  {
    if (length < 1) return;
    ConfigManager& configManager = ConfigManager::getInstance();
    bool telemetry3rd = payload[0] - '0';
    Serial.print(F("Third party telemetry (sat owners,  satnog... ) "));  if (telemetry3rd) Serial.println(F("ON")); else Serial.println(F("OFF"));
    configManager.setTelemetry3rd(telemetry3rd);
  }

  if (!strcmp(command, commandFrame))
  {
    uint8_t frameNumber = atoi(strtok(NULL, "/"));
    radio.remoteTextFrame((char*)payload, length, frameNumber);
  }

    // Remote_Status       -m "[1]"     -t fossa/g4lile0/test_G4lile0_new/data/remote/status
  if (!strcmp(command, commandStatus))
    radio.remote_status((char*)payload, length);

  // ######################################################
  // ############## Remote tune commands ##################
  // ######################################################
  if (ConfigManager::getInstance().getRemoteTune() && global)
      return;

  // Remote_Begin_Lora       -m "[437.7,125.0,11,8,18,11,120,8,0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/begin_lora
  if (!strcmp(command, commandBeginLora))
    radio.remote_begin_lora((char*)payload, length);

  // Remote_Begin_FSK       -m "[433.5,100.0,10.0,250.0,10,100,16,0,0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/begin_fsk
  if (!strcmp(command, commandBeginFSK))
    radio.remote_begin_fsk((char*)payload, length);

  // Remote_Frequency       -m "[434.8]" -t fossa/g4lile0/test_G4lile0_new/data/remote/freq
  if (!strcmp(command, commandFreq))
    radio.remote_freq((char*)payload, length);

  // Remote_Lora_Bandwidth  -m "[250]" -t fossa/g4lile0/test_G4lile0_new/data/remote/bw
  if (!strcmp(command, commandBw))
    radio.remote_bw((char*)payload, length);

  // Remote_spreading factor -m "[11]" -t fossa/g4lile0/test_G4lile0_new/data/remote/sf 
  if (!strcmp(command, commandSf))
    radio.remote_sf((char*)payload, length);

  // Remote_Coding rate       -m "[8]" -t fossa/g4lile0/test_G4lile0_new/data/remote/cr
  if (!strcmp(command, commandCr))
    radio.remote_cr((char*)payload, length);

  // Remote_Crc               -m "[0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/crc
  if (!strcmp(command, commandCrc))
    radio.remote_crc((char*)payload, length);

  // Remote_LoRa_syncword          -m "[8,1,2,3,4,5,6,7,8,9]" -t fossa/g4lile0/test_G4lile0_new/data/remote/lsw
  if (!strcmp(command, commandCrc))
    radio.remote_lsw((char*)payload, length);

  // Remote_Force LDRO        -m "[0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fldro
  if (!strcmp(command, commandFldro))
    radio.remote_fldro((char*)payload, length);

  // Remote_auto LDRO        -m "[0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/aldro
  if (!strcmp(command, commandAldro))
    radio.remote_aldro((char*)payload, length);

  // Remote_Preamble Lenght   -m "[8]" -t fossa/g4lile0/test_G4lile0_new/data/remote/pl
  if (!strcmp(command, commandPl))
    radio.remote_pl((char*)payload, length);

  // Remote_FSK_BitRate      -m "[250]" -t fossa/g4lile0/test_G4lile0_new/data/remote/br
  if (!strcmp(command, commandBr))
    radio.remote_br((char*)payload, length);

  // Remote_FSK_FrequencyDeviation  -m "[10.0]" -t fossa/g4lile0/test_G4lile0_new/data/remote/Fd
  if (!strcmp(command, commandFd))
    radio.remote_fd((char*)payload, length);

  // Remote_FSK_RxBandwidth       -m "[125]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fbw
  if (!strcmp(command, commandFbw))
    radio.remote_fbw((char*)payload, length);

  // Remote_FSK_syncword          -m "[8,1,2,3,4,5,6,7,8,9]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fsw
  if (!strcmp(command, commandFsw))
    radio.remote_fsw((char*)payload, length);

  // Remote_FSK_Set_OOK + DataShapingOOK(only sx1278)     -m "[1,2]" -t fossa/g4lile0/test_G4lile0_new/data/remote/fok
  if (!strcmp(command, commandFook))
    radio.remote_fook((char*)payload, length);

  // Remote_Satellite_Name       -m "[\"FossaSat-3\" , 46494 ]" -t fossa/g4lile0/test_G4lile0_new/data/remote/sat
  if (!strcmp(command, commandSatPos))
    radio.remote_sat((char*)payload, length);

  // GOD MODE  With great power comes great responsibility!
  // SPIsetRegValue  (only sx1278)     -m "[1,2,3,4,5]" -t fossa/g4lile0/test_G4lile0_new/data/remote/SPIsetRegValue
  if (!strcmp(command, commandSPIsetRegValue))
    radio.remote_SPIsetRegValue((char*)payload, length);

  // SPIwriteRegister  (only sx1278)     -m "[1,2]" -t fossa/g4lile0/test_G4lile0_new/data/remote/SPIwriteRegister
  if (!strcmp(command, commandSPIwriteRegister))
    radio.remote_SPIwriteRegister((char*)payload, length);

  // SPIreadRegister            -m "[1]" -t fossa/g4lile0/test_G4lile0_new/data/remote/SPIreadRegister
  if (!strcmp(command, commandSPIreadRegister))
    radio.remote_SPIreadRegister((char*)payload, length);

  // Remote_Status       -m "[1]"     -t fossa/g4lile0/test_G4lile0_new/data/remote/JSON
  if (!strcmp(command, commandBatchConf))
  {
    Serial.println("JSON");
    DynamicJsonDocument doc(2048);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
    deserializeJson(doc, payload, length);
    JsonObject root = doc.as<JsonObject>();

    for (JsonPair kv : root)
    {
      Serial.print(kv.key().c_str());
      Serial.print("  ");
      Serial.print(kv.value().as<char*>());
      Serial.print("  ");
      Serial.println(strlen(kv.value().as<char*>()));

      const char* key = kv.key().c_str();
      char* value = (char*)kv.value().as<char*>();
      size_t len = strlen(value);

      if (!strcmp(key, commandCrc))        
        radio.remote_crc(value, len);

      else if (!strcmp(key, commandFreq))
        radio.remote_freq(value, len);

      else if (!strcmp(key, commandBeginLora))
        radio.remote_begin_lora(value, len);

      else if (!strcmp(key, commandBw))
        radio.remote_bw(value, len);

      else if (!strcmp(key, commandSf))
        radio.remote_sf(value, len);

      else if (!strcmp(key, commandLsw))
        radio.remote_lsw(value, len);

      else if (!strcmp(key, commandFldro))
        radio.remote_fldro(value, len);

      else if (!strcmp(key, commandAldro))
        radio.remote_aldro(value, len);

      else if (!strcmp(key, commandPl))
        radio.remote_pl(value, len);

      else if (!strcmp(key, commandBeginFSK))
        radio.remote_begin_fsk(value, len);

      else if (!strcmp(key, commandBr))
        radio.remote_br(value, len);

      else if (!strcmp(key, commandFd))
        radio.remote_fd(value, len);

      else if (!strcmp(key, commandFbw))
        radio.remote_fbw(value, len);

      else if (!strcmp(key, commandFsw))
        radio.remote_fsw(value, len);

      else if (!strcmp(key, commandFook))
        radio.remote_fook(value, len);

      else if (!strcmp(key, commandSat))
        radio.remote_sat(value, len);

      else if (!strcmp(key, commandSPIsetRegValue))
        radio.remote_SPIsetRegValue(value, len);

      else if (!strcmp(key, commandSPIwriteRegister))
        radio.remote_SPIwriteRegister(value, len);

      else if (!strcmp(key, commandSPIreadRegister))
        radio.remote_SPIreadRegister(value, len);
    }
  }
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
