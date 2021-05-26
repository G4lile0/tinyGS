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
#include "../Logger/Logger.h"
#include "../Radio/Radio.h"
#include "ArduinoJson.h"
#if ARDUINOJSON_USE_LONG_LONG == 0 && !PLATFORMIO
#error "Using Arduino IDE is not recommended, please follow this guide https://github.com/G4lile0/tinyGS/wiki/Arduino-IDE or edit /ArduinoJson/src/ArduinoJson/Configuration.hpp and amend to #define ARDUINOJSON_USE_LONG_LONG 1 around line 68"
#endif

ConfigManager::ConfigManager()
    : IotWebConf2(thingName, &dnsServer, &server, initialApPassword, configVersion), server(80), gsConfigHtmlFormatProvider(*this), boards({
                                                                                                                                        //OLED_add, OLED_SDA,  OLED_SCL, OLED_RST, PROG_BUTTON, BOARD_LED, L_SX127X?, L_NSS, L_DI00, L_DI01, L_BUSSY, L_RST,  L_MISO, L_MOSI, L_SCK, L_TCXO_V, BOARD
                                                                                                                                        {0x3c, 4, 15, 16, 0, 25, 1, 18, 26, 12, 0, 14, 19, 27, 5, 0.0f, "433Mhz HELTEC WiFi LoRA 32 V1"}, // @4m1g0
                                                                                                                                        {0x3c, 4, 15, 16, 0, 25, 1, 18, 26, 12, 0, 14, 19, 27, 5, 0.0f, "863-928Mhz HELTEC WiFi LoRA 32 V1"},
                                                                                                                                        {0x3c, 4, 15, 16, 0, 25, 1, 18, 26, 35, 0, 14, 19, 27, 5, 0.0f, "433Mhz HELTEC WiFi LoRA 32 V2"}, // @4m1g0
                                                                                                                                        {0x3c, 4, 15, 16, 0, 25, 1, 18, 26, 35, 0, 14, 19, 27, 5, 0.0f, "863-928Mhz HELTEC WiFi LoRA 32 V2"},
                                                                                                                                        {0x3c, 4, 15, 16, 0, 2, 1, 18, 26, 0, 0, 14, 19, 27, 5, 0.0f, "433Mhz  TTGO LoRa 32 v1"},       // @g4lile0
                                                                                                                                        {0x3c, 4, 15, 16, 0, 2, 1, 18, 26, 0, 0, 14, 19, 27, 5, 0.0f, "868-915Mhz TTGO LoRa 32 v1"},    //
                                                                                                                                        {0x3c, 21, 22, 16, 0, 22, 1, 18, 26, 33, 0, 14, 19, 27, 5, 0.0f, "433 Mhz TTGO LoRA 32 v2"},    // @TCRobotics
                                                                                                                                        {0x3c, 21, 22, 16, 0, 22, 1, 18, 26, 33, 0, 14, 19, 27, 5, 0.0f, "868-915Mhz TTGO LoRA 32 v2"}, //
                                                                                                                                        {0x3c, 21, 22, 16, 39, 22, 1, 18, 26, 33, 32, 14, 19, 27, 5, 0.0f, "433Mhz T-BEAM + OLED"},
                                                                                                                                        {0x3c, 21, 22, 16, 39, 22, 1, 18, 26, 33, 32, 14, 19, 27, 5, 0.0f, "868-915Mhz T-BEAM + OLED"},
                                                                                                                                        {0x3c, 21, 22, 16, 0, 25, 0, 5, 0, 27, 26, 14, 19, 23, 18, 0.0f, "Custom ESP32 Wroom + SX126x (Crystal)"},                       // @4m1g0, @lillefyr
                                                                                                                                        {0x3c, 21, 22, 16, 0, 25, 0, 18, 0, 33, 32, 14, 19, 27, 5, 0.0f, "TTGO LoRa 32 V2 Modified with module SX126x (crystal)"},       // @TCRobotics
                                                                                                                                        {0x3c, 21, 22, 16, 0, 25, 0, 5, 0, 2, 13, 26, 19, 23, 18, 1.6f, "Custom ESP32 Wroom + SX126x DRF1268T (TCX0) (5, 2, 26, 13)"},   // @sdey76
                                                                                                                                        {0x3c, 21, 22, 16, 0, 25, 0, 5, 0, 26, 12, 14, 19, 23, 18, 1.6f, "Custom ESP32 Wroom + SX126x DRF1268T (TCX0) (5, 26, 14, 12)"}, // @imants
                                                                                                                                        {0x3c, 21, 22, 16, 38, 22, 1, 18, 26, 33, 0, 14, 19, 27, 5, 0.0f, "T-BEAM V1.0 + OLED"},                                         // @fafu
                                                                                                                                        {0x3c, 21, 22, 16, 0, 2, 0, 5, 0, 34, 32, 14, 19, 27, 18, 1.6f, "433Mhz FOSSA 1W Ground Station"},                               // @jgromes
                                                                                                                                        {0x3c, 21, 22, 16, 0, 2, 0, 5, 0, 34, 32, 14, 19, 27, 18, 1.6f, "868-915Mhz FOSSA 1W Ground Station"},                           // @jgromes
                                                                                                                                    })
{
  server.on(ROOT_URL, [this] { handleRoot(); });
  server.on(CONFIG_URL, [this] { handleConfig(); });
  server.on(DASHBOARD_URL, [this] { handleDashboard(); });
  server.on(RESTART_URL, [this] { handleRestart(); });
  server.on(REFRESH_CONSOLE_URL, [this] { handleRefreshConsole(); });
  setupUpdateServer(
      [this](const char *updatePath) { httpUpdater.setup(&server, updatePath); },
      [this](const char *userName, char *password) { httpUpdater.updateCredentials(userName, password); });
  setHtmlFormatProvider(&gsConfigHtmlFormatProvider);
  formValidatorStd = std::bind(&ConfigManager::formValidator, this, std::placeholders::_1);
  setFormValidator(formValidatorStd);
  setConfigSavedCallback([this] { configSavedCallback(); });
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

  groupAdvanced.addItem(&boardTemplateParam);
  groupAdvanced.addItem(&modemParam);
  groupAdvanced.addItem(&advancedConfigParam);
  addParameterGroup(&groupAdvanced);
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

  // uint64_t time = millis(); // TODO: add current time
  String s = String(FPSTR(IOTWEBCONF_HTML_HEAD));
  s += "<style>" + String(FPSTR(IOTWEBCONF_HTML_STYLE_INNER)) + "</style>";
  s += "<style>" + String(FPSTR(IOTWEBCONF_DASHBOARD_STYLE_INNER)) + "</style>";
  s += "<script>" + String(FPSTR(IOTWEBCONF_CONSOLE_SCRIPT)) + "</script>";
  s += FPSTR(IOTWEBCONF_HTML_HEAD_END);
  s += FPSTR(IOTWEBCONF_DASHBOARD_BODY_INNER);
  s += String(FPSTR(LOGO)) + "<br />";
  s += F("</table></div><div class=\"card\"><h3>Groundstation Status</h3><table>");
  s += "<tr><td>Name </td><td>" + String(getThingName()) + "</td></tr>";
  s += "<tr><td>Version </td><td>" + String(status.version) + "</td></tr>";
  s += "<tr><td>MQTT Server </td><td>" + String(status.mqtt_connected ? "<span class='G'>CONNECTED</span>" : "<span class='R'>NOT CONNECTED</span>") + "</td></tr>";
  s += "<tr><td>WiFi </td><td>" + String(WiFi.isConnected() ? "<span class='G'>CONNECTED</span>" : "<span class='R'>NOT CONNECTED</span>") + "</td></tr>";
  s += "<tr><td>Radio </td><td>" + String(Radio::getInstance().isReady() ? "<span class='G'>READY</span>" : "<span class='R'>NOT READY</span>") + "</td></tr>";
  s += "<tr><td>Test Mode </td><td>" + String(getTestMode() ? "ENABLED" : "DISABLED") + "</td></tr>";
  //s += "<tr><td>Uptime </td><td>" + // process and update in js + "</td></tr>";
  s += F("</table></div>");
  s += F("<div class=\"card\"><h3>Modem Configuration</h3><table>");
  s += "<tr><td>Listening to </td><td>" + String(status.modeminfo.satellite) + "</td></tr>";
  s += "<tr><td>Modulation </td><td>" + String(status.modeminfo.modem_mode) + "</td></tr>";
  s += "<tr><td>Frequency </td><td>" + String(status.modeminfo.frequency) + "</td></tr>";
  if (status.modeminfo.modem_mode == "LoRa")
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
  s += "<tr><td>Frequency error </td><td>" + String(status.lastPacketInfo.frequencyerror) + "</td></tr>";
  s += "<tr><td colspan=\"2\" style=\"text-align:center;\">" + String(status.lastPacketInfo.crc_error ? "CRC ERROR!" : "") + "</td></tr>";
  s += F("</table></div>");
  s += FPSTR(IOTWEBCONF_CONSOLE_BODY_INNER);
  s += "<br /><button style='max-width: 1080px;' onclick=\"window.location.href='" + String(ROOT_URL) + "';\">Go Back</button><br /><br />";
  s += FPSTR(IOTWEBCONF_HTML_END);

  s.replace("{v}", FPSTR(TITLE_TEXT));

  server.sendHeader("Content-Length", String(s.length()));
  server.send(200, "text/html; charset=UTF-8", s);
}

void ConfigManager::handleRefreshConsole()
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

  uint32_t counter = 0;

  String svalue = server.arg("c1");
  if (svalue.length())
  {
    Log::console(PSTR("COMMAND: %s"), svalue.c_str());

    if (strcmp(svalue.c_str(), "p") == 0)
    {
      if (!getAllowTx())
      {
        Log::console(PSTR("Radio transmission is not allowed by config! Check your config on the web panel and make sure transmission is allowed by local regulations"));
      }
      else
      {
        static long lastTestPacketTime = 0;
        if (millis() - lastTestPacketTime < 20 * 1000)
        {
          Log::console(PSTR("Please wait a few seconds to send another test packet."));
        }
        else
        {
          Radio &radio = Radio::getInstance();
          radio.sendTestPacket();
          lastTestPacketTime = millis();
          Log::console(PSTR("Sending test packet to nearby stations!"));
        }
      }
    }
    else
    {
      Log::console(PSTR("%s"), F("Command still not supported in web serial console!"));
    }
  }

  char stmp[8];
  String s = server.arg("c2");
  strlcpy(stmp, s.c_str(), sizeof(stmp));
  if (strlen(stmp))
  {
    counter = atoi(stmp);
  }
  server.client().flush();
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  server.sendHeader(F("Pragma"), F("no-cache"));
  server.sendHeader(F("Expires"), F("-1"));
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, F("text/plain"), "");
  server.sendContent(String((uint8_t)Log::getLogIdx()) + "\n");
  if (counter != Log::getLogIdx())
  {
    if (!counter)
    {
      counter = Log::getLogIdx();
    }
    do
    {
      char *tmp;
      size_t len;
      Log::getLog(counter, &tmp, &len);
      if (len)
      {
        char stemp[len + 1];
        memcpy(stemp, tmp, len);
        stemp[len - 1] = '\n';
        stemp[len] = '\0';
        server.sendContent(stemp);
      }
      counter++;
      counter &= 0xFF;
      if (!counter)
      {
        counter++;
      } // Skip log index 0 as it is not allowed
    } while (counter != Log::getLogIdx());
  }

  server.sendContent("");
  server.client().stop();
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

bool ConfigManager::formValidator(iotwebconf2::WebRequestWrapper *webRequestWrapper)
{
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

  for (const char *c = name.c_str(); *c != '\0'; c++)
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
  latitude[0] = '\0';
  longitude[0] = '\0';
  oledBright[0] = '\0';
  allowTx[0] = '\0';
  remoteTune[0] = '\0';
  telemetry3rd[0] = '\0';
  testMode[0] = '\0';
  autoUpdate[0] = '\0';
  boardTemplate[0] = '\0';
  modemStartup[0] = '\0';
  advancedConfig[0] = '\0';

  saveConfig();
}

void ConfigManager::resetModemConfig()
{
  strncpy(modemStartup, MODEM_DEFAULT, MODEM_LEN);
  saveConfig();
  ESP.restart();
}

boolean ConfigManager::init()
{
  boolean validConfig = IotWebConf2::init();

  // when wifi credentials are set but we are not able to connect (maybe wrong credentials)
  // we fall back to AP mode during 2 minutes after which we try to connect again and repeat.
  setApTimeoutMs(atoi(AP_TIMEOUT_MS));

  // no board selected
  if (!strcmp(board, ""))
  {
    boardDetection();
  }

  if (strlen(advancedConfig))
    parseAdvancedConf();

  parseModemStartup();

  strcpy(savedThingName, this->getThingName());
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
  Log::error(PSTR("Automatic board detection running... "));
  for (uint8_t ite = 0; ite < ((sizeof(boards) / sizeof(boards[0]))); ite++)
  {
    Serial.print(boards[ite].BOARD);
    pinMode(boards[ite].OLED__RST, OUTPUT);
    digitalWrite(boards[ite].OLED__RST, LOW);
    delay(50);
    digitalWrite(boards[ite].OLED__RST, HIGH);
    Wire.begin(boards[ite].OLED__SDA, boards[ite].OLED__SCL);
    Wire.beginTransmission(boards[ite].OLED__address);
    if (!Wire.endTransmission())
    {
      Log::error(PSTR("Compatible OLED FOUND"));
      itoa(ite, board, 10);
      break;
    }
    else
    {
      Log::error(PSTR("Not Compatible board found, please select it manually on the web config panel"));
    }
  }
}

void ConfigManager::printConfig()
{
  Log::debug(PSTR("MQTT Port: %u\nMQTT Server: %s\nMQTT Pass: %s\nLatitude: %f\nLongitude: %f"), getMqttPort(), getMqttServer(), getMqttPass(), getLatitude(), getLongitude());
  Log::debug(PSTR("tz: %s\nboard: %u --> %s\nOLED Bright: %u\nTX %s"), getTZ(), getBoard(), boards[getBoard()].BOARD.c_str(), getOledBright(), getAllowTx() ? "Enable" : "Disable");
  Log::debug(PSTR("Remote Tune %\nSend telemetry to third party %s"), getRemoteTune() ? "Allowed" : "Blocked", getTelemetry3rd() ? "Allowed" : "Blocked");
  Log::debug(PSTR("Test mode %s\nAuto Update %s"), getTestMode() ? "Enable" : "Disable", getAutoUpdate() ? "Enable" : "Disable");
}

void ConfigManager::configSavedCallback()
{
  // If the station name changes we have to restart as it is considered a different station
  if (strcmp(getThingName(), savedThingName))
  {
    ESP.restart();
  }

  if (!remoteSave)
  {
    forceApMode(false);
    parseModemStartup();
    MQTT_Client::getInstance().scheduleRestart();
  }

  parseAdvancedConf();

  remoteSave = false;
}

void ConfigManager::parseAdvancedConf()
{
  if (!strlen(advancedConfig))
    return;

  size_t size = 512;
  DynamicJsonDocument doc(size);
  deserializeJson(doc, (const char *)advancedConfig);

  if (doc.containsKey(F("dmode")))
  {
    Log::setLogLevel(doc["dmode"]);
  }

  if (doc.containsKey(F("flipOled")))
  {
    advancedConf.flipOled = doc["flipOled"];
  }

  if (doc.containsKey(F("dnOled")))
  {
    advancedConf.dnOled = doc["dnOled"];
  }

  if (doc.containsKey(F("lowPower")))
  {
    advancedConf.lowPower = doc["lowPower"];
  }
}

void ConfigManager::parseModemStartup()
{
  size_t size = JSON_ARRAY_SIZE(10) + 10 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(16) + JSON_ARRAY_SIZE(8) + JSON_ARRAY_SIZE(8) + 64;
  DynamicJsonDocument doc(size);
  DeserializationError error = deserializeJson(doc, (const char *)modemStartup);

  if (error.code() != DeserializationError::Ok || !doc.containsKey("mode"))
  {
    Log::console(PSTR("ERROR: Your modem config is invalid. Resetting to default"));
    resetModemConfig();
    return;
  }

  ModemInfo &m = status.modeminfo;
  m.modem_mode = doc["mode"].as<String>();
  strcpy(m.satellite, doc["sat"].as<char *>());
  m.NORAD = doc["NORAD"];

  if (m.modem_mode == "LoRa")
  {
    m.frequency = doc["freq"];
    m.bw = doc["bw"];
    m.sf = doc["sf"];
    m.cr = doc["cr"];
    m.sw = doc["sw"];
    m.power = doc["pwr"];
    m.preambleLength = doc["pl"];
    m.gain = doc["gain"];
    m.crc = doc["crc"];
    m.fldro = doc["fldro"];
  }
  else
  {
    m.frequency = doc["freq"];
    m.bw = doc["bw"];
    m.bitrate = doc["br"];
    m.freqDev = doc["fd"];
    m.power = doc["pwr"];
    m.preambleLength = doc["pl"];
    m.OOK = doc["ook"];
    m.swSize = doc["fsw"].size();
    for (int i = 0; i < 8; i++)
    {
      if (i < m.swSize)
        m.fsw[i] = doc["fsw"][i];
      else
        m.fsw[i] = 0;
    }
  }

  // packets Filter
  uint8_t filterSize = doc["filter"].size();
  for (int i = 0; i < 8; i++)
  {
    if (i < filterSize)
      status.modeminfo.filter[i] = doc["filter"][i];
    else
      status.modeminfo.filter[i] = 0;
  }

  if (Radio::getInstance().isReady())
    Radio::getInstance().begin();
}
