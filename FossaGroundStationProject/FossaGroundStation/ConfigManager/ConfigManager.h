#include "IotWebConf.h"
#include "logos.h"

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

const char TITLE_TEXT[] PROGMEM = "FOSSA Ground Satation Configuration";


constexpr auto thingName = "test_GroundStation";
constexpr auto initialApPassword = "";
constexpr auto configVersion = "v0.0.1";

#define MQTT_DEFAULT_SERVER "fossa.apaluba.com"
#define MQTT_DEFAULT_PORT  "8883"

constexpr auto AP_TIMEOUT_MS = "300000";

enum boards {
  HELTEC_V1 = 0,
  HELTEC_V2,
  TTGO_V1,
  TTGO_V2,
  ESP32_SX126X
};

#include "htmlOptions.h"

class GSConfigHtmlFormatProvider : public IotWebConfHtmlFormatProvider
{
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
      return FPSTR(IOTWEBCONF_HTML_FORM_TZ_PARAM);
    }

    if (!strcmp(type, "board")) {
      return FPSTR(IOTWEBCONF_HTML_FORM_BOARD_PARAM);
    }

    return IotWebConfHtmlFormatProvider::getFormParam(type);
  }
};


class ConfigManager : public IotWebConf
{
public:
  ConfigManager();
  void resetAPConfig();
  void resetAllConfig();
  boolean init();

  uint16_t getMqttPort();
  const char* getMqttServer();
  const char* getMqttPass();
  float getLatitude();
  float getLongitude();
  const char* getTZ();
  uint8_t getBoard();

private:
  void handleRoot();
  void handleDashboard();
  bool formValidator();
  
  std::function<boolean()> formValidatorStd;
  DNSServer dnsServer;
  WebServer server;
  HTTPUpdateServer httpUpdater;
  GSConfigHtmlFormatProvider gsConfigHtmlFormatProvider;

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

