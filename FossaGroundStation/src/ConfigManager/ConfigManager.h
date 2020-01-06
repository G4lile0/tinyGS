/*
  ConfigManager.h - Config Manager class
  
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
#ifndef ConfigManager_h
#define ConfigManager_h

#include "IotWebConf2.h"
#include "logos.h"
#include <Wire.h>
#include "htmlOptions.h"

constexpr auto STATION_NAME_LENGTH = 21;
constexpr auto COORDINATE_LENGTH = 10;
constexpr auto MQTT_SERVER_LENGTH = 31;
constexpr auto MQTT_PORT_LENGTH = 6;
constexpr auto MQTT_USER_LENGTH = 31;
constexpr auto MQTT_PASS_LENGTH = 31;
constexpr auto SSID_LENGTH = 32;
constexpr auto PASS_LENGTH = 64;
constexpr auto TZ_LENGTH = 40;
constexpr auto BOARD_LENGTH = 3;

constexpr auto ROOT_URL = "/";
constexpr auto CONFIG_URL = "/config";
constexpr auto DASHBOARD_URL = "/dashboard";
constexpr auto UPDATE_URL = "/firmware";
constexpr auto RESTART_URL = "/restart";

const char TITLE_TEXT[] PROGMEM = "FOSSA Ground Satation Configuration";


constexpr auto thingName = "test_GroundStation";
constexpr auto initialApPassword = "";
constexpr auto configVersion = "0.01"; //max 4 chars

#define MQTT_DEFAULT_SERVER "fossa.apaluba.com"
#define MQTT_DEFAULT_PORT  "8883"

constexpr auto AP_TIMEOUT_MS = "300000";

enum boardNum {
  HELTEC_V1 = 0,
  HELTEC_V2,
  TTGO_V1,
  TTGO_V2,
  TBEAM_OLED,
  ESP32_SX126X_XTAL,
  TTGO_V2_SX126X_XTAL,
  ESP32_SX126X_TXC0_1,
  ESP32_SX126X_TXC0_2,

  NUM_BOARDS //this line always has to be the last one
};

typedef struct {
   uint8_t  OLED__address;
   uint8_t  OLED__SDA;
   uint8_t  OLED__SCL;
   uint8_t  OLED__RST;
   uint8_t  PROG__BUTTON;
   uint8_t  BOARD_LED;
   bool     L_SX127X;
   uint8_t  L_NSS;        // CS
   uint8_t  L_DI00;        
   uint8_t  L_DI01;
   uint8_t  L_BUSSY;        
   uint8_t  L_RST;
   uint8_t  L_MISO;
   uint8_t  L_MOSI;        
   uint8_t  L_SCK;
   float    L_TCXO_V;
   String   BOARD;
} board_type;

class ConfigManager : public IotWebConf2
{
public:
  ConfigManager();
  void resetAPConfig();
  void resetAllConfig();
  boolean init();
  void printConfig();

  uint16_t getMqttPort() { return (uint16_t) atoi(mqttPort); }
  const char* getMqttServer() { return mqttServer; }
  const char* getMqttUser() { return mqttUser; }
  const char* getMqttPass() { return mqttPass; }
  float getLatitude() { return atof(latitude); }
  float getLongitude() { return atof(longitude); }
  const char* getTZ() { return tz; }
  uint8_t getBoard() { return atoi(board); }
  const char* getWiFiSSID() { return getWifiSsidParameter()->valueBuffer; }

  bool isApMode() { return (getState() != IOTWEBCONF_STATE_CONNECTING && getState() != IOTWEBCONF_STATE_ONLINE); }
  bool isConnected() { return getState() == IOTWEBCONF_STATE_ONLINE; };
  board_type getBoardConfig(){ return boards[getBoard()]; }

private:
  class GSConfigHtmlFormatProvider : public IotWebConfHtmlFormatProvider
  {
  public:
    GSConfigHtmlFormatProvider(ConfigManager& x) : configManager(x) { }
  protected:
    String getScriptInner() override
    {
      return
        IotWebConfHtmlFormatProvider::getScriptInner();
        //String(FPSTR(CUSTOMHTML_SCRIPT_INNER));
    }
    String getBodyInner() override
    {
      return
        String(FPSTR(LOGO)) +
        IotWebConfHtmlFormatProvider::getBodyInner();
    }

    String getFormParam(const char* type) override
    {
      if (!strcmp(type, "TZ")) {
        String select = String(FPSTR(IOTWEBCONF_HTML_FORM_TZ_PARAM));
        String find = "value='" + String(configManager.tz) + "'";
        select.replace(find, find + " selected");
        Serial.println(find);
        return select;
      }

      if (!strcmp(type, "board")) {
        String select = String(FPSTR(IOTWEBCONF_HTML_FORM_BOARD_PARAM));
        String find = "value='" + String(configManager.board) + "'";
        select.replace(find, find + " selected");
        return select;
      }

      return IotWebConfHtmlFormatProvider::getFormParam(type);
    }

    ConfigManager& configManager;
  };

  void handleRoot();
  void handleDashboard();
  void handleRestart();
  bool formValidator();
  void boardDetection();
  
  std::function<boolean()> formValidatorStd;
  DNSServer dnsServer;
  WebServer server;
  HTTPUpdateServer httpUpdater;
  GSConfigHtmlFormatProvider gsConfigHtmlFormatProvider;
  board_type boards[NUM_BOARDS]; 

  char latitude[COORDINATE_LENGTH] = "";
  char longitude[COORDINATE_LENGTH] = "";
  char tz[TZ_LENGTH] = "";
  char mqttServer[MQTT_SERVER_LENGTH] = MQTT_DEFAULT_SERVER;
  char mqttPort[MQTT_PORT_LENGTH] = MQTT_DEFAULT_PORT;
  char mqttUser[MQTT_USER_LENGTH] = "";
  char mqttPass[MQTT_PASS_LENGTH] = "";
  char board[BOARD_LENGTH] = "";

  IotWebConfParameter latitudeParam = IotWebConfParameter("Latitude (will be public)", "lat", latitude, COORDINATE_LENGTH, "number", NULL, NULL, "required min='-180' max='180' step='0.001'");
  IotWebConfParameter longitudeParam = IotWebConfParameter("Longitude (will be public)", "lng", longitude, COORDINATE_LENGTH, "number", NULL, NULL, "required min='-180' max='180' step='0.001'");
  IotWebConfParameter tzParam = IotWebConfParameter("Time Zone", "tz", tz, TZ_LENGTH, "TZ", NULL, NULL);

  IotWebConfSeparator mqttSeparator = IotWebConfSeparator("Mqtt credentials (get them <a href='https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q'>here</a>)");
  IotWebConfParameter mqttServerParam = IotWebConfParameter("Server address", "mqtt_server", mqttServer, MQTT_SERVER_LENGTH, "text", NULL, MQTT_DEFAULT_SERVER, "required type=\"text\" maxlength=30");
  IotWebConfParameter mqttPortParam = IotWebConfParameter("Server Port", "mqtt_port", mqttPort, MQTT_PORT_LENGTH, "number", NULL, MQTT_DEFAULT_PORT, "required type=\"number\" min=\"0\" max=\"65536\" step=\"1\"");
  IotWebConfParameter mqttUserParam = IotWebConfParameter("MQTT Username", "mqtt_user", mqttUser, MQTT_USER_LENGTH, "text", NULL, NULL, "required type=\"text\" maxlength=30");
  IotWebConfParameter mqttPassParam = IotWebConfParameter("MQTT Password", "mqtt_pass", mqttPass, MQTT_PASS_LENGTH, "text", NULL, NULL, "required type=\"text\" maxlength=30");

  IotWebConfSeparator separatorBoard = IotWebConfSeparator("Board config");
  IotWebConfParameter boardParam = IotWebConfParameter("Board type", "board", board, BOARD_LENGTH, "board", NULL, NULL);
};

#endif