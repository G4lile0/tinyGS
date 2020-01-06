/*
  ConfigManager.cpp - Config Manager class
  
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

#include "ConfigManager.h"

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
  })
{
  server.on(ROOT_URL, [this]{ handleRoot(); });
  server.on(CONFIG_URL, [this]{ handleConfig(); });
  server.on(DASHBOARD_URL, [this]{ handleDashboard(); });
  server.on(RESTART_URL, [this]{ handleRestart(); });
  setupUpdateServer(&httpUpdater);
  setHtmlFormatProvider(&gsConfigHtmlFormatProvider);
  formValidatorStd = std::bind(&ConfigManager::formValidator, this);
  setFormValidator(formValidatorStd);
  skipApStartup();
  

  // Customize own parameters
  getThingNameParameter()->label = "GroundStation Name";
  getApPasswordParameter()->label = "GroundStation dashboard password";

  addParameter(&latitudeParam);
  addParameter(&longitudeParam);
  addParameter(&tzParam);
  addParameter(&mqttSeparator);
  addParameter(&mqttServerParam);
  addParameter(&mqttPortParam);
  addParameter(&mqttUserParam);
  addParameter(&mqttPassParam);
  addParameter(&separatorBoard);
  addParameter(&boardParam);
}

void ConfigManager::handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
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
  s += FPSTR(IOTWEBCONF_HTML_HEAD_END);
  s += FPSTR(IOTWEBCONF_HTML_BODY_INNER);
  s += String(FPSTR(LOGO)) + "<br />";
  s += "We are still working on this feature. It will be ready soon.<br /><br/>";
  s += "<button onclick=\"window.location.href='" + String(ROOT_URL) + "';\">Go Back</button><br /><br />";
  s += FPSTR(IOTWEBCONF_HTML_END);

  s.replace("{v}", FPSTR(TITLE_TEXT));

  server.sendHeader("Content-Length", String(s.length()));
  server.send(200, "text/html; charset=UTF-8", s);
}

void ConfigManager::handleRestart() {
  if (getState() == IOTWEBCONF_STATE_ONLINE)
  {
    // -- Authenticate
    if (!server.authenticate(
            IOTWEBCONF_ADMIN_USER_NAME, getApPasswordParameter()->valueBuffer))
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

bool ConfigManager::formValidator()
{
  Serial.println("Validating form.");
  boolean valid = true;

  int l = 4;//server.arg(stringParam.getId()).length();
  if (l < 3)
  {
    //stringParam.errorMessage = "Please provide at least 3 characters for this test!";
    valid = false;
  }

  return valid;
}

void ConfigManager::resetAPConfig() {
  getWifiSsidParameter()->valueBuffer[0] = '\0';
  getWifiPasswordParameter()->valueBuffer[0] = '\0';
  strncpy(getApPasswordParameter()->valueBuffer, initialApPassword, IOTWEBCONF_WORD_LEN);
  strncpy(getThingNameParameter()->valueBuffer, thingName, IOTWEBCONF_WORD_LEN);
  strncpy(getThingName(), thingName, IOTWEBCONF_WORD_LEN);
  configSave();
}

void ConfigManager::resetAllConfig(){
  getWifiSsidParameter()->valueBuffer[0] = '\0';
  getWifiPasswordParameter()->valueBuffer[0] = '\0';
  strncpy(getApPasswordParameter()->valueBuffer, initialApPassword, IOTWEBCONF_WORD_LEN);
  strncpy(getThingNameParameter()->valueBuffer, thingName, IOTWEBCONF_WORD_LEN);
  strncpy(getThingName(), thingName, IOTWEBCONF_WORD_LEN);
  strncpy(mqttPortParam.valueBuffer, MQTT_DEFAULT_PORT, MQTT_PORT_LENGTH);
  strncpy(mqttServerParam.valueBuffer, MQTT_DEFAULT_SERVER, MQTT_SERVER_LENGTH);
  mqttUserParam.valueBuffer[0] = '\0';
  mqttPassParam.valueBuffer[0] = '\0';

  configSave();
}

boolean ConfigManager::init() {
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

void ConfigManager::boardDetection() {
  // List all compatible boards configuration
  Serial.println(F("\nSupported boards:"));
  for (uint8_t ite=0; ite<((sizeof(boards)/sizeof(boards[0])));ite++) {
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
  }
  
  // test OLED configuration
  Serial.println(F("Seaching for a compatible BOARD"));
  for (uint8_t ite=0; ite<((sizeof(boards)/sizeof(boards[0])));ite++) {
    Serial.print(boards[ite].BOARD);
    pinMode(boards[ite].OLED__RST,OUTPUT);
    digitalWrite(boards[ite].OLED__RST, LOW);     
    delay(50);
    digitalWrite(boards[ite].OLED__RST, HIGH);
    Wire.begin (boards[ite].OLED__SDA, boards[ite].OLED__SCL);
    Wire.beginTransmission(boards[ite].OLED__address);
    if (!Wire.endTransmission()) { 
      Serial.println(F("  Compatible OLED FOUND")); 
      itoa(ite, board, 10);
      break;
    }
    else {
      Serial.println(F("  Not Compatible"));
    } 
  }
}

void ConfigManager::printConfig() {
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
}