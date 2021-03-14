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

#include "MQTTClient.h"
#include "mqtt_client.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"
#include "../Radio/Radio.h"
#include "../OTA/OTA.h"
#include "../Logger/Logger.h"

// MQTT_Client::MQTT_Client() 
// : PubSubClient(espClient)
// {
//     espClient.setCACert (DSTroot_CA);
// }

void MQTT_Client::loop() {
//   if (!connected() && millis() - lastConnectionAtempt > reconnectionInterval)
//   {
    //lastConnectionAtempt = millis();
    //connectionAtempts++;
    // status.mqtt_connected = false;
    // lastPing = millis();
    // reconnect();
//   }
//   else
//   {
//     connectionAtempts = 0;
//     status.mqtt_connected = true;
//   }

//   if (connectionAtempts > connectionTimeout)
//   {
//     Log::console(PSTR("Unable to connect to MQTT Server after many atempts. Restarting..."));
//     ESP.restart();
//   }

//   PubSubClient::loop();

  unsigned long now = millis();
  if (now - lastPing > pingInterval && connected())
  {
    lastPing = now;
    publish(buildTopic(teleTopic, topicPing).c_str(), "1");
  }
}

// void MQTT_Client::reconnect()
// {
//   ConfigManager& configManager = ConfigManager::getInstance();
//   uint64_t chipId = ESP.getEfuseMac();
//   char clientId[13];
//   sprintf(clientId, "%04X%08X",(uint16_t)(chipId>>32), (uint32_t)chipId);

//   Log::console(PSTR("Attempting MQTT connection..."));
//   Log::console(PSTR("If this is taking more than expected, connect to the config panel on the ip: %s to review the MQTT connection credentials."), WiFi.localIP().toString().c_str());
//   if (connect(clientId, configManager.getMqttUser(), configManager.getMqttPass(), buildTopic(teleTopic, topicStatus).c_str(), 2, false, "0")) {
//     Log::console(PSTR("Connected to MQTT!"));
//     status.mqtt_connected = true;
//     subscribeToAll();
//     sendWelcome();
//   }
//   else {
//     status.mqtt_connected = false;
//     Log::console(PSTR("failed, rc=%i"), state());
//   }
// }

String MQTT_Client::buildTopic(const char* baseTopic, const char* cmnd)
{
  ConfigManager& configManager = ConfigManager::getInstance();
  String topic = baseTopic;
  topic.replace("%user%", configManager.getMqttUser());
  topic.replace("%station%", configManager.getThingName());
  topic.replace("%cmnd%", cmnd);

  return topic;
}

void MQTT_Client::subscribeToAll () {
    esp_mqtt_client_subscribe (mqtt_client, buildTopic (globalTopic, "#").c_str(), 0);
    esp_mqtt_client_subscribe (mqtt_client, buildTopic (cmndTopic, "#").c_str (), 0);
}

void MQTT_Client::sendWelcome()
{
  ConfigManager& configManager = ConfigManager::getInstance();
  time_t now;
  time(&now);

  uint64_t chipId = ESP.getEfuseMac();
  char clientId[13];
  sprintf(clientId, "%04X%08X",(uint16_t)(chipId>>32), (uint32_t)chipId);


  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(14) + 22 + 20 +1;
  DynamicJsonDocument doc(capacity);
  JsonArray station_location = doc.createNestedArray("station_location");
  station_location.add(configManager.getLatitude());
  station_location.add(configManager.getLongitude());
  doc["version"] = status.version;
  doc["git_version"] = status.git_version;
  doc["board"] = configManager.getBoard();
  doc["mac"] = clientId;
  doc["tx"] = configManager.getAllowTx();
  doc["remoteTune"] = configManager.getRemoteTune();
  doc["telemetry3d"] = configManager.getTelemetry3rd();
  doc["test"] = configManager.getTestMode();
  doc["unix_GS_time"] = now;
  doc["autoUpdate"] = configManager.getAutoUpdate();
  doc["local_ip"]= WiFi.localIP().toString();
  doc["modem_conf"].set(configManager.getModemStartup());
  doc["boardTemplate"].set(configManager.getBoardTemplate());

  char buffer[1048];
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

  const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(21);
  DynamicJsonDocument doc(capacity);
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
    doc["rxBw"] = status.modeminfo.bw;
  }

  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["unix_GS_time"] = now;
  doc["usec_time"] = (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;
  doc["time_offset"] = status.time_offset;
  doc["crc_error"] = status.lastPacketInfo.crc_error;
  doc["data"] = packet.c_str();
  doc["NORAD"] = status.modeminfo.NORAD;
  doc["test"] = configManager.getTestMode();

  char buffer[1536];
  serializeJson(doc, buffer);
  Log::debug(PSTR("%s"), buffer);
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
    doc["rxBw"] = status.modeminfo.bw;
  }

  doc["pl"] = status.modeminfo.preambleLength;
  doc["CRC"] = status.modeminfo.crc;
  doc["FLDRO"] = status.modeminfo.fldro;
  doc["OOK"] = status.modeminfo.OOK;

  doc["rssi"] = status.lastPacketInfo.rssi;
  doc["snr"] = status.lastPacketInfo.snr;
  doc["frequency_error"] = status.lastPacketInfo.frequencyerror;
  doc["crc_error"] = status.lastPacketInfo.crc_error;
  doc["unix_GS_time"] = now;
  doc["usec_time"] = (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;
  doc["time_offset"] = status.time_offset;
    
  char buffer[1024];
  serializeJson(doc, buffer);
  publish(buildTopic(statTopic, topicStatus).c_str(), buffer, false);
}

void MQTT_Client::manageMQTTData(char *topic, uint8_t *payload, unsigned int length)
{
  Radio& radio = Radio::getInstance();

  bool global = true;
  char* command;
  strtok(topic, "/");  // tinygs
  if (strcmp(strtok(NULL, "/"), "global")) // user
  {
    global = false;
    strtok(NULL, "/"); // station
  }
  strtok(NULL, "/"); // cmnd
  command = strtok(NULL, "/");
  uint16_t result = 0xFF;

  if (!strcmp(command, commandSatPos)) {
    manageSatPosOled((char*)payload, length);
    return; // no ack
  }

  if (!strcmp(command, commandReset))
    ESP.restart();

  if (!strcmp(command, commandUpdate)) {
    OTA::update();
    return; // no ack
  }

  if (!strcmp(command, commandTest))
  {
    if (length < 1) return;
    ConfigManager& configManager = ConfigManager::getInstance();
    bool test = payload[0] - '0';
    Log::console(PSTR("Set Test Mode to %s"), test ? F("ON") : F("OFF"));
    configManager.setTestMode(test);
    result = 0;
  }

  if (!strcmp(command, commandRemoteTune))
  {
    if (length < 1) return;
    ConfigManager& configManager = ConfigManager::getInstance();
    bool tune = payload[0] - '0';
    Log::console(PSTR("Set Remote Tune to %s"), tune ? F("ON") : F("OFF"));
    configManager.setRemoteTune(tune);
    result = 0;
  }

  if (!strcmp(command, commandRemoteTune))
  {
    if (length < 1) return;
    ConfigManager& configManager = ConfigManager::getInstance();
    bool telemetry3rd = payload[0] - '0';
    Log::console(PSTR("Send rx to third parties %s"), telemetry3rd ? F("ON") : F("OFF"));
    configManager.setTelemetry3rd(telemetry3rd);
    result = 0;
  }

  if (!strcmp(command, commandFrame))
  {
    uint8_t frameNumber = atoi(strtok(NULL, "/"));
    DynamicJsonDocument doc(JSON_ARRAY_SIZE(5) * 15 + JSON_ARRAY_SIZE(15));
    deserializeJson(doc, payload, length);
    status.remoteTextFrameLength[frameNumber] = doc.size();
    Log::debug(PSTR("Received frame: %u"), status.remoteTextFrameLength[frameNumber]);
  
    for (uint8_t n=0; n<status.remoteTextFrameLength[frameNumber];n++)
    {
      status.remoteTextFrame[frameNumber][n].text_font = doc[n][0];
      status.remoteTextFrame[frameNumber][n].text_alignment = doc[n][1];
      status.remoteTextFrame[frameNumber][n].text_pos_x = doc[n][2];
      status.remoteTextFrame[frameNumber][n].text_pos_y = doc[n][3];
      String text = doc[n][4];
      status.remoteTextFrame[frameNumber][n].text = text;  
      
      Log::debug(PSTR("Text: %u Font: %u Alig: %u Pos x: %u Pos y: %u -> %s"), n, 
                                      status.remoteTextFrame[frameNumber][n].text_font, 
                                      status.remoteTextFrame[frameNumber][n].text_alignment, 
                                      status.remoteTextFrame[frameNumber][n].text_pos_x,
                                      status.remoteTextFrame[frameNumber][n].text_pos_y,
                                      status.remoteTextFrame[frameNumber][n].text.c_str());
    }

    result = 0;
  }

  if (!strcmp(command, commandStatus)) 
  {
    uint8_t mode = payload[0] - '0';
    Log::debug(PSTR("Remote status requested: %u"), mode);     // right now just one mode
    sendStatus();
    return;
  }

  // ######################################################
  // ############## Remote tune commands ##################
  // ######################################################
  if (ConfigManager::getInstance().getRemoteTune() && global)
    return;
  
  if (!strcmp(command, commandBegin))
  {
    char buff[length+1];
    memcpy(buff, payload, length);
    buff[length] = '\0';
    Log::debug(PSTR("%s"), buff);
    ConfigManager::getInstance().setModemStartup(buff);
  }

  // Remote_Begin_Lora [437.7,125.0,11,8,18,11,120,8,0]
  if (!strcmp(command, commandBeginLora))
    result = radio.remote_begin_lora((char*)payload, length);

  // Remote_Begin_FSK [433.5,100.0,10.0,250.0,10,100,16,0,0]
  if (!strcmp(command, commandBeginFSK))
    result = radio.remote_begin_fsk((char*)payload, length);

  if (!strcmp(command, commandFreq))
    result = radio.remote_freq((char*)payload, length);

  if (!strcmp(command, commandBw))
    result = radio.remote_bw((char*)payload, length);

  if (!strcmp(command, commandSf))
    result = radio.remote_sf((char*)payload, length);

  if (!strcmp(command, commandCr))
    result = radio.remote_cr((char*)payload, length);

  if (!strcmp(command, commandCrc))
    result = radio.remote_crc((char*)payload, length);

  // Remote_LoRa_syncword [8,1,2,3,4,5,6,7,8,9]
  if (!strcmp(command, commandCrc))
    result = radio.remote_lsw((char*)payload, length);

  if (!strcmp(command, commandFldro))
    result = radio.remote_fldro((char*)payload, length);

  if (!strcmp(command, commandAldro))
    result = radio.remote_aldro((char*)payload, length);

  if (!strcmp(command, commandPl))
    result = radio.remote_pl((char*)payload, length);

  if (!strcmp(command, commandBr))
    result = radio.remote_br((char*)payload, length);

  if (!strcmp(command, commandFd))
    result = radio.remote_fd((char*)payload, length);

  if (!strcmp(command, commandFbw))
    result = radio.remote_fbw((char*)payload, length);

  // Remote_FSK_syncword [8,1,2,3,4,5,6,7,8,9]
  if (!strcmp(command, commandFsw))
    result = radio.remote_fsw((char*)payload, length);

  // Remote_FSK_Set_OOK + DataShapingOOK(only sx1278) [1,2]
  if (!strcmp(command, commandFook))
    result = radio.remote_fook((char*)payload, length);

  // Remote_Satellite_Name [\"FossaSat-3\" , 46494 ]
  if (!strcmp(command, commandSat))
  {
    remoteSatCmnd((char*)payload, length);
    result = 0;
  }

  // GOD MODE  With great power comes great responsibility!
  // SPIsetRegValue  (only sx1278) [1,2,3,4,5]
  if (!strcmp(command, commandSPIsetRegValue))
    result = radio.remote_SPIsetRegValue((char*)payload, length);

  // SPIwriteRegister  (only sx1278) [1,2]
  if (!strcmp(command, commandSPIwriteRegister))
  {
    radio.remote_SPIwriteRegister((char*)payload, length);
    result = 0;
  }

  if (!strcmp(command, commandSPIreadRegister) && !global)
    result = radio.remote_SPIreadRegister((char*)payload, length);

  if (!strcmp(command, commandBatchConf))
  {
    Log::debug(PSTR("BatchConfig"));
    DynamicJsonDocument doc(2048);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
    deserializeJson(doc, payload, length);
    JsonObject root = doc.as<JsonObject>();
    result = 0;

    for (JsonPair kv : root)
    {
      const char* key = kv.key().c_str();
      char* value = (char*)kv.value().as<char*>();
      size_t len = strlen(value);
      Log::debug(PSTR("%s %s %u"), key, value, len);

      if (!strcmp(key, commandCrc))        
        result = radio.remote_crc(value, len);

      else if (!strcmp(key, commandFreq))
        result = radio.remote_freq(value, len);

      else if (!strcmp(key, commandBeginLora))
        result = radio.remote_begin_lora(value, len);

      else if (!strcmp(key, commandBw))
        result = radio.remote_bw(value, len);

      else if (!strcmp(key, commandSf))
        result = radio.remote_sf(value, len);

      else if (!strcmp(key, commandLsw))
        result = radio.remote_lsw(value, len);

      else if (!strcmp(key, commandFldro))
        result = radio.remote_fldro(value, len);

      else if (!strcmp(key, commandAldro))
        result = radio.remote_aldro(value, len);

      else if (!strcmp(key, commandPl))
        result = radio.remote_pl(value, len);

      else if (!strcmp(key, commandBeginFSK))
        result = radio.remote_begin_fsk(value, len);

      else if (!strcmp(key, commandBr))
        result = radio.remote_br(value, len);

      else if (!strcmp(key, commandFd))
        result = radio.remote_fd(value, len);

      else if (!strcmp(key, commandFbw))
        result = radio.remote_fbw(value, len);

      else if (!strcmp(key, commandFsw))
        result = radio.remote_fsw(value, len);

      else if (!strcmp(key, commandFook))
        result = radio.remote_fook(value, len);

      else if (!strcmp(key, commandSat))
        remoteSatCmnd(value, len);

      else if (!strcmp(key, commandSPIsetRegValue))
        result = radio.remote_SPIsetRegValue(value, len);

      else if (!strcmp(key, commandSPIwriteRegister))
        radio.remote_SPIwriteRegister(value, len);
      
      if (result) // there was an error
      {
        Log::debug(PSTR("Error ocurred during batch config!!"));
        break;
      }
    }
  }

  if (!global)
    publish(buildTopic(statTopic, command).c_str(), (uint8_t*)&result, 2U, false);
}

void MQTT_Client::manageSatPosOled(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(60);
  deserializeJson(doc, payload, payload_len);
  status.satPos[0] = doc[0];
  status.satPos[1] = doc[1];
}

void MQTT_Client::remoteSatCmnd(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload, payload_len);
  String satellite = doc[0];
  uint32_t NORAD = doc[1];
  status.modeminfo.NORAD = NORAD;
  status.modeminfo.satellite = satellite;

  Log::debug(PSTR("Listening Satellite: %s NORAD: %u"), satellite, NORAD);
}

// Helper class to use as a callback
void MQTT_Client::manageMQTTDataCallback (void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
    //char* topic, uint8_t* payload, unsigned int length)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    MQTT_Client mqttclient = MQTT_Client::getInstance ();

    switch (event_id) {
    case MQTT_EVENT_CONNECTED:
        mqttclient.connectionAtempts = 0;
        status.mqtt_connected = true;
        mqttclient.mqtt_connected = true;
        Log::console (PSTR ("Connected to MQTT!"));
        mqttclient.subscribeToAll ();
        mqttclient.sendWelcome ();

        break;
    case MQTT_EVENT_DISCONNECTED:
        mqttclient.connectionAtempts++;
        status.mqtt_connected = false;
        mqttclient.mqtt_connected = false;
        mqttclient.lastPing = millis ();
        if (mqttclient.connectionAtempts > mqttclient.connectionTimeout) {
            Log::console (PSTR ("Unable to connect to MQTT Server after many atempts. Restarting..."));
            ESP.restart ();
        }
        break;
    case MQTT_EVENT_SUBSCRIBED:
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        break;
    case MQTT_EVENT_PUBLISHED:
        break;
    case MQTT_EVENT_DATA:
    {
        char* topic = (char*)malloc (event->topic_len + 1);
        memcpy (topic, event->topic, event->topic_len);
        topic[event->topic_len] = 0;
        unsigned int length = event->data_len;
        uint8_t* payload = (uint8_t*)(event->data);

        Log::debug (PSTR ("Received MQTT message: %s : %.*s"), topic, length, payload);
        mqttclient.manageMQTTData (topic, payload, length);
    }
        break;
    case MQTT_EVENT_ERROR:
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            Log::debug (PSTR("Last error code reported from esp-tls: 0x%x"), event->error_handle->esp_tls_last_esp_err);
            Log::debug (PSTR ("Last tls stack error number: 0x%x"), event->error_handle->esp_tls_stack_err);
            Log::debug (PSTR ("Last captured errno : %d (%s)"), event->error_handle->esp_transport_sock_errno,
                        strerror (event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            Log::debug (PSTR ("MQTT connection refused error: 0x%x"), event->error_handle->connect_return_code);
        } else {
            Log::debug (PSTR ("Unknown error type: 0x%x"), event->error_handle->error_type);
        }
        break;
    case MQTT_EVENT_BEFORE_CONNECT:
        Log::console (PSTR ("Attempting MQTT connection..."));
        Log::console (PSTR ("If this is taking more than expected, connect to the config panel on the ip: %s to review the MQTT connection credentials."), WiFi.localIP ().toString ().c_str ());
        mqttclient.lastConnectionAtempt = millis ();
        break;
    default:
        //Log::console (PSTR("Unknown event id:%d"), event->event_id);
        break;
    }
}

void MQTT_Client::begin()
{
    ConfigManager& configManager = ConfigManager::getInstance ();

    //ConfigManager& configManager = ConfigManager::getInstance ();
    uint64_t chipId = ESP.getEfuseMac ();
    sprintf (clientId, "%04X%08X", (uint16_t)(chipId >> 32), (uint32_t)chipId);
    
    randomSeed (micros ());

    mqtt_cfg.host = configManager.getMqttServer ();
    mqtt_cfg.port = configManager.getMqttPort ();
    mqtt_cfg.client_id = clientId;
#ifdef SECURE_MQTT
    mqtt_cfg.cert_pem = DSTroot_CA;
    mqtt_cfg.transport = MQTT_TRANSPORT_OVER_SSL;
#else
    mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
#endif
    mqtt_cfg.username = configManager.getMqttUser ();
    mqtt_cfg.password = configManager.getMqttPass ();
    mqtt_cfg.lwt_topic = buildTopic (teleTopic, topicStatus).c_str ();
    mqtt_cfg.lwt_msg = "0";
    mqtt_cfg.lwt_msg_len = 1;
    mqtt_cfg.lwt_qos = 0;
    mqtt_cfg.lwt_retain = false;
    mqtt_cfg.buffer_size = MQTT_MAX_PACKET_SIZE;
    //mqtt_cfg.keepalive = 30;

    esp_err_t result;
        
    if (!(mqtt_client = esp_mqtt_client_init (&mqtt_cfg))) {
        Log::console (PSTR ("Error configuring MQTT client"));
    }
    if (result = esp_mqtt_client_register_event (mqtt_client, MQTT_EVENT_ANY, manageMQTTDataCallback, this)) {
        Log::console (PSTR ("Error registering MQTT event handler: %s"), esp_err_to_name (result));
    }

    if (result = esp_mqtt_client_start (mqtt_client)) {
        Log::console (PSTR ("Error starting MQTT client: %s"), esp_err_to_name(result));
    }
    
}

boolean MQTT_Client::publish (const char* topic, const char* payload) {
    return publish (topic, (const uint8_t*)payload, payload ? strnlen (payload, MQTT_MAX_PACKET_SIZE) : 0, false);
}

boolean MQTT_Client::publish (const char* topic, const char* payload, boolean retained) {
    return publish (topic, (const uint8_t*)payload, payload ? strnlen (payload, MQTT_MAX_PACKET_SIZE) : 0, retained);
}

boolean MQTT_Client::publish (const char* topic, const uint8_t* payload, unsigned int plength) {
    return publish (topic, payload, plength, false);
}

boolean MQTT_Client::publish (const char* topic, const uint8_t* payload, unsigned int plength, boolean retained) {
    if (!topic || !payload) {
        return false;
    }

    if (!strlen (topic)) {
        return false;
    }

    if (!mqtt_connected) {
        return false;
    }
    
    return !esp_mqtt_client_publish (mqtt_client, topic, (char*)payload, plength, 0, retained);
}