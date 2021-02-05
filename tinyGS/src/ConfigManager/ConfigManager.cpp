/*
  ConfigManager.cpp - Config Manager class
  
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

#include "ConfigManager.h"
#include "../Mqtt/MQTT_Client.h"

ConfigManager::ConfigManager()
: IotWebConf2(thingName, &dnsServer, &server, initialApPassword, configVersion)
, server(80)
, gsConfigHtmlFormatProvider(*this)
, boards({
  //OLED_add, OLED_SDA,  OLED_SCL, OLED_RST, PROG_BUTTON, BOARD_LED, L_SX127X?, L_NSS, L_DI00, L_DI01, L_BUSSY, L_RST,  L_MISO, L_MOSI, L_SCK, L_TCXO_V, BOARD 
  {      0x3c,        4,        15,       16,           0,        25,      true,    18,     26,     12,      0,    14,      19,     27,     5,     0.0f, "HELTEC WiFi LoRA 32 V1" }, // @4m1g0
  {      0x3c,        4,        15,       16,           0,        25,      true,    18,     26,     35,      0,    14,      19,     27,     5,     0.0f, "HELTEC WiFi LoRA 32 V2" }, // @4m1g0
  {      0x3c,        4,        15,       16,           0,         2,      true,    18,     26,      0,      0,    14,      19,     27,     5,     0.0f, "TTGO LoRa 32 v1"        }, // @g4lile0
  {      0x3c,       21,        22,       16,           0,        22,      true,    18,     26,     33,      0,    14,      19,     27,     5,     0.0f, "TTGO LoRA 32 v2"        }, // @TCRobotics
  {      0x3c,       21,        22,       16,          39,        22,      true,    18,     26,     33,     32,    14,      19,     27,     5,     0.0f, "T-BEAM + OLED"        }, 
  {      0x3c,       21,        22,       16,           0,        25,     false,     5,      0,     27,     26,    14,      19,     23,    18,     0.0f, "Custom ESP32 Wroom + SX126x (Crystal)"  }, // @4m1g0, @lillefyr
  {      0x3c,       21,        22,       16,           0,        25,     false,    18,      0,     33,     32,    14,      19,     27,     5,     0.0f, "TTGO LoRa 32 V2 Modified with module SX126x (crystal)"  },// @TCRobotics
  {      0x3c,       21,        22,       16,           0,        25,     false,     5,      0,      2,     13,    26,      19,     23,    18,     1.6f, "Custom ESP32 Wroom + SX126x DRF1268T (TCX0) (5, 2, 26, 13)"  }, // @sdey76
  {      0x3c,       21,        22,       16,           0,        25,     false,     5,      0,     26,     12,    14,      19,     23,    18,     1.6f, "Custom ESP32 Wroom + SX126x DRF1268T (TCX0) (5, 26, 14, 12)"  }, // @imants
  {      0x3c,       21,        22,       16,          38,        22,      true,    18,     26,     33,      0,    14,      19,     27,     5,     0.0f, "T-BEAM V1.0 + OLED"     }, // @fafu

  })
{
  server.on(ROOT_URL, [this]{ handleRoot(); });
  server.on(CONFIG_URL, [this]{ handleConfig(); });
  server.on(DASHBOARD_URL, [this]{ handleDashboard(); });
  server.on(RESTART_URL, [this]{ handleRestart(); });
  setupUpdateServer(
    [this](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [this](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });
  setHtmlFormatProvider(&gsConfigHtmlFormatProvider);
  formValidatorStd = std::bind(&ConfigManager::formValidator, this, std::placeholders::_1);
  setFormValidator(formValidatorStd);
  setConfigSavedCallback([this]{ configSavedCallback(); });
  skipApStartup();
  

  // Customize own parameters
  getThingNameParameter()->label = "GroundStation Name (will be seen on the map)";
  getApPasswordParameter()->label = "Password for this dashboard (user is <b>admin</b>)";

  addSystemParameter(&latitudeParam);
  addSystemParameter(&longitudeParam);
  addSystemParameter(&tzParam);

  groupMqtt.addItem(&mqttServerParam);
  groupMqtt.addItem(&mqttPortParam);
  groupMqtt.addItem(&mqttUserParam);
  groupMqtt.addItem(&mqttPassParam);
  addParameterGroup(&groupMqtt);

  groupBoardConfig.addItem(&boardParam);
  groupBoardConfig.addItem(&oledBrightParam);
  groupBoardConfig.addItem(&AllowTxParam);
  groupBoardConfig.addItem(&remoteTuneParam);
  groupBoardConfig.addItem(&telemetry3rdParam);
  groupBoardConfig.addItem(&testParam);
  groupBoardConfig.addItem(&autoUpdateParam);
  addParameterGroup(&groupBoardConfig);
}

void ConfigManager::handleRoot()
{
  // -- Let IotWebConf2 test and handle captive portal requests.
  if (handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }

  String s = String(FPSTR(IOTWEBCONF_HTML_HEAD));
  s += "<style>" + String(FPSTR(IOTWEBCONF_HTML_STYLE_INNER)) + "</style>";
  s += FPSTR(IOTWEBCONF_HTML_HEAD_END);
  s += FPSTR(IOTWEBCONF_HTML_BODY_INNER);
  s += String(FPSTR(LOGO)) + "<br />";
  s += "<button onclick=\"window.location.href='" + String(DASHBOARD_URL) + "';\">Station dashboard</button><br /><br />";
  s += "<button onclick=\"window.location.href='" + String(CONFIG_URL) + "';\">Configure parameters</button><br /><br />";
  s += "<button onclick=\"window.location.href='" + String(UPDATE_URL) + "';\">Upload new version</button><br /><br />";
  s += "<button onclick=\"window.location.href='" + String(RESTART_URL) + "';\">Restart Station</button><br /><br />";
  s += FPSTR(IOTWEBCONF_HTML_END);

  s.replace("{v}", FPSTR(TITLE_TEXT));

  server.sendHeader("Content-Length", String(s.length()));
  server.send(200, "text/html; charset=UTF-8", s);
}

void ConfigManager::handleDashboard()
{
  String s = String(FPSTR(IOTWEBCONF_HTML_HEAD));
  s += "<style>" + String(FPSTR(IOTWEBCONF_HTML_STYLE_INNER)) + "</style>";
  s += "<style>" + String(FPSTR(IOTWEBCONF_DASHBOARD_STYLE_INNER)) + "</style>";
  s += FPSTR(IOTWEBCONF_HTML_HEAD_END);
  s += FPSTR(IOTWEBCONF_DASHBOARD_BODY_INNER);
  s += String(FPSTR(LOGO)) + "<br />";
  s += F("</table></div><div class=\"card\"><h3>Groundstation Status</h3><table>");
  s += "<tr><td>Name </td><td>" + String(getThingName()) + "</td></tr>";
  s += "<tr><td>Version </td><td>" + String(status.version) + "</td></tr>";
  s += "<tr><td>MQTT Server </td><td>" + String(status.mqtt_connected?"<span class='G'>CONNECTED</span>":"<span class='R'>NOT CONNECTED</span>") + "</td></tr>";
  s += "<tr><td>WiFi </td><td>" + String(WiFi.isConnected()?"<span class='G'>CONNECTED</span>":"<span class='R'>NOT CONNECTED</span>") + "</td></tr>";
  s += "<tr><td>Test Mode </td><td>" + String(getTestMode()?"ENABLED":"DISABLED") + "</td></tr>";
  s += F("</table></div>");
  s += F("<div class=\"card\"><h3>Modem Configuration</h3><table>");
  s += "<tr><td>Listening to </td><td>" + String(status.modeminfo.satellite) + "</td></tr>";
  s += "<tr><td>Modulation </td><td>" + String(status.modeminfo.modem_mode) + "</td></tr>";
  s += "<tr><td>Frequency </td><td>" + String(status.modeminfo.frequency) + "</td></tr>";
  if (status.modeminfo.satellite == "LoRa")
  {
    s += "<tr><td>Spreading Factor </td><td>" + String(status.modeminfo.sf) + "</td></tr>";
    s += "<tr><td>Coding Rate </td><td>" + String(status.modeminfo.cr) + "</td></tr>";
    s += "<tr><td>Bandwidth </td><td>" + String(status.modeminfo.bw) + "</td></tr>";
  }
  else
  {
    s += "<tr><td>Bitrate </td><td>" + String(status.modeminfo.bitrate) + "</td></tr>";
    s += "<tr><td>Frequency dev </td><td>" + String(status.modeminfo.freqDev) + "</td></tr>";
    s += "<tr><td>Bandwidth </td><td>" + String(status.modeminfo.bw) + "</td></tr>";
  }
  s += F("</table></div><div class=\"card\"><h3>Last Packet Received</h3><table>");
  s += "<tr><td>Received at </td><td>" + String(status.lastPacketInfo.time) + "</td></tr>";
  s += "<tr><td>Signal RSSI </td><td>" + String(status.lastPacketInfo.rssi) + "</td></tr>";
  s += "<tr><td>Signal SNR </td><td>" + String(status.lastPacketInfo.snr) + "</td></tr>";
  s += "<tr><td>Frequiency error </td><td>" + String(status.lastPacketInfo.frequencyerror) + "</td></tr>";
  s += "<tr><td colspan=\"2\" style=\"text-align:center;\">" + String(status.lastPacketInfo.crc_error?"CRC ERROR!":"") + "</td></tr>";
  s += F("</table></div>");
  s += FPSTR(IOTWEBCONF_CONSOLE_BODY_INNER);
  s += "<br /><button style='max-width: 1080px;' onclick=\"window.location.href='" + String(ROOT_URL) + "';\">Go Back</button><br /><br />";
  s += FPSTR(IOTWEBCONF_HTML_END);

  s.replace("{v}", FPSTR(TITLE_TEXT));

  server.sendHeader("Content-Length", String(s.length()));
  server.send(200, "text/html; charset=UTF-8", s);
}

void ConfigManager::handleRestart()
{
  if (getState() == IOTWEBCONF_STATE_ONLINE)
  {
    // -- Authenticate
    if (!server.authenticate(IOTWEBCONF_ADMIN_USER_NAME, getApPasswordParameter()->valueBuffer))
    {
      IOTWEBCONF_DEBUG_LINE(F("Requesting authentication."));
      server.requestAuthentication();
      return;
    }
  }


  String s = String(FPSTR(IOTWEBCONF_HTML_HEAD));
  s += "<style>" + String(FPSTR(IOTWEBCONF_HTML_STYLE_INNER)) + "</style>";
  s += "<meta http-equiv=\"refresh\" content=\"8; url=/\">";
  s += FPSTR(IOTWEBCONF_HTML_HEAD_END);
  s += FPSTR(IOTWEBCONF_HTML_BODY_INNER);
  s += String(FPSTR(LOGO)) + "<br />";
  s += "Ground Station is restarting...<br /><br/>";
  s += FPSTR(IOTWEBCONF_HTML_END);

  s.replace("{v}", FPSTR(TITLE_TEXT));

  server.sendHeader("Content-Length", String(s.length()));
  server.send(200, "text/html; charset=UTF-8", s);
  delay(100);
  ESP.restart();
}

bool ConfigManager::formValidator(iotwebconf2::WebRequestWrapper* webRequestWrapper)
{
  Serial.println("Validating form.");

  String name = webRequestWrapper->arg(this->getThingNameParameter()->getId());
  
  if (name.length() < 3)
  {
    this->getThingNameParameter()->errorMessage = "Your ground station name must have more then 3 characters and less then 25 and should be unique.";
    return false;
  }

  if (!strcmp(name.c_str(), thingName))
  {
    this->getThingNameParameter()->errorMessage = "Please, change your station name to something unique. Be creative!";
    return false;
  }

  for (const char* c = name.c_str(); *c != '\0'; c++)
  {
    if (!((*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'Z') || *c == '_' || (*c >= 'a' && *c <= 'z')))
    {
      this->getThingNameParameter()->errorMessage = "Allowed characters are: [0-9 A-Z a-z _]";
      return false;
    }
  }

  return true;
}

void ConfigManager::resetAPConfig()
{
  getWifiSsidParameter()->valueBuffer[0] = '\0';
  getWifiPasswordParameter()->valueBuffer[0] = '\0';
  strncpy(getApPasswordParameter()->valueBuffer, initialApPassword, IOTWEBCONF_WORD_LEN);
  strncpy(getThingNameParameter()->valueBuffer, thingName, IOTWEBCONF_WORD_LEN);
  strncpy(getThingName(), thingName, IOTWEBCONF_WORD_LEN);

  saveConfig();
}

void ConfigManager::resetAllConfig()
{
  getWifiSsidParameter()->valueBuffer[0] = '\0';
  getWifiPasswordParameter()->valueBuffer[0] = '\0';
  strncpy(getApPasswordParameter()->valueBuffer, initialApPassword, IOTWEBCONF_WORD_LEN);
  strncpy(getThingNameParameter()->valueBuffer, thingName, IOTWEBCONF_WORD_LEN);
  strncpy(getThingName(), thingName, IOTWEBCONF_WORD_LEN);
  strncpy(mqttPortParam.valueBuffer, MQTT_DEFAULT_PORT, MQTT_PORT_LENGTH);
  strncpy(mqttServerParam.valueBuffer, MQTT_DEFAULT_SERVER, MQTT_SERVER_LENGTH);
  mqttUserParam.valueBuffer[0] = '\0';
  mqttPassParam.valueBuffer[0] = '\0';
  latitude[0]  = '\0';
  longitude[0]  = '\0';
  oledBright[0]  = '\0';
  allowTx[0]  = '\0';
  remoteTune[0]  = '\0';
  telemetry3rd[0]  = '\0';
  testMode[0]  = '\0';
  autoUpdate[0]  = '\0';

  saveConfig();
}

boolean ConfigManager::init()
{
  boolean validConfig = IotWebConf2::init();

  // when wifi credentials are set but we are not able to connect (maybe wrong credentials)
  // we fall back to AP mode during 2 minutes after which we try to connect again and repeat.
  setApTimeoutMs(atoi(AP_TIMEOUT_MS));

  // no board selected
  if (!strcmp(board, "")){
    boardDetection();
  }

  return validConfig;
}

void ConfigManager::boardDetection()
{
  // List all compatible boards configuration
  /*Serial.println(F("\nSupported boards:"));
  for (uint8_t ite=0; ite<((sizeof(boards)/sizeof(boards[0])));ite++)
  {
    Serial.println("");
    Serial.println(boards[ite].BOARD);
    Serial.print(F(" OLED: Adrs 0x"));    Serial.print(boards[ite].OLED__address,HEX);
    Serial.print(F(" SDA:"));      Serial.print(boards[ite].OLED__SDA);
    Serial.print(F(" SCL:"));      Serial.print(boards[ite].OLED__SCL);
    Serial.print(F(" RST:"));      Serial.print(boards[ite].OLED__RST);
    Serial.print(F(" BUTTON:"));   Serial.println(boards[ite].PROG__BUTTON);
    Serial.print(F(" Lora Module "));
    if (boards[ite].L_SX127X) {Serial.print(F("SX1278 ")); } else {Serial.print(F("SX1268:"));} ;
    Serial.print(F(" NSS:"));      Serial.print(boards[ite].L_NSS);
    Serial.print(F(" MOSI:"));     Serial.print(boards[ite].L_MOSI);
    Serial.print(F(" MISO:"));     Serial.print(boards[ite].L_MISO);
    Serial.print(F(" SCK:"));      Serial.print(boards[ite].L_SCK);
      
    if (boards[ite].L_DI00) {Serial.print(F(" DI00:")); Serial.print(boards[ite].L_DI00);}
    if (boards[ite].L_DI01) {Serial.print(F(" DI01:")); Serial.print(boards[ite].L_DI01);}
    if (boards[ite].L_BUSSY) {Serial.print(F(" BUSSY:")); Serial.print(boards[ite].L_BUSSY);}
    Serial.println("");   
  }*/
  
  // test OLED configuration
  Serial.println(F("Automatic board detection running... "));
  for (uint8_t ite=0; ite<((sizeof(boards)/sizeof(boards[0])));ite++)
  {
    Serial.print(boards[ite].BOARD);
    pinMode(boards[ite].OLED__RST,OUTPUT);
    digitalWrite(boards[ite].OLED__RST, LOW);     
    delay(50);
    digitalWrite(boards[ite].OLED__RST, HIGH);
    Wire.begin (boards[ite].OLED__SDA, boards[ite].OLED__SCL);
    Wire.beginTransmission(boards[ite].OLED__address);
    if (!Wire.endTransmission())
    { 
      Serial.println(F("  Compatible OLED FOUND")); 
      itoa(ite, board, 10);
      break;
    }
    else 
    {
      Serial.println(F("  Not Compatible board found, please select it manually on the web config panel"));
    } 
  }
}

void ConfigManager::printConfig()
{
  Serial.print(F("MQTT Port: "));
  Serial.println(getMqttPort());
  Serial.print(F("MQTT Server: "));
  Serial.println(getMqttServer());
  Serial.print(F("MQTT Pass: "));
  Serial.println(getMqttPass());
  Serial.print(F("Latitude: "));
  Serial.println(getLatitude());
  Serial.print(F("Longitude: "));
  Serial.println(getLongitude());
  Serial.print(F("tz: "));
  Serial.println(getTZ());
  Serial.print(F("board: "));
  Serial.print(getBoard());
  Serial.print(F(" -->  "));
  Serial.println(boards[getBoard()].BOARD);
  Serial.print(F("OLED Bright: "));
  Serial.println(getOledBright());
  Serial.print(F("TX "));
  Serial.println(getAllowTx() ? "Enable" : "Disable");
  Serial.print(F("Remote Tune "));
  Serial.println(getRemoteTune() ? "Allowed" : "Blocked");
  Serial.print(F("Send telemetry to third party "));
  Serial.println(getTelemetry3rd() ? "Allowed" : "Blocked");
  Serial.print(F("Test mode "));
  Serial.println(getTestMode()  ? "Enable" : "Disable");
  Serial.print(F("Auto Update "));
  Serial.println(getAutoUpdate()  ? "Enable" : "Disable");
}

void ConfigManager::configSavedCallback()
{
  MQTT_Client& mqtt = MQTT_Client::getInstance();

  if (mqtt.connected()) // already running and connected
    mqtt.sendWelcome();
}
