#include "IotWebConf.h"
#include "logos.h"

constexpr auto STATION_NAME_LENGTH = 21;
constexpr auto MQTT_SERVER_LENGTH = 31;
constexpr auto MQTT_USER_LENGTH = 31;
constexpr auto MQTT_PASS_LENGTH = 31;
constexpr auto SSID_LENGTH = 32;
constexpr auto PASS_LENGTH = 64;
constexpr auto TZ_LENGTH = 40;

constexpr auto ROOT_URL = "/";
constexpr auto CONFIG_URL = "/config";
constexpr auto DASHBOARD_URL = "/dashboard";
constexpr auto UPDATE_URL = "/firmware";

const char TITLE_TEXT[] PROGMEM = "FOSSA Ground Satation Configuration";


constexpr auto thingName = "GroundStation";
constexpr auto initialApPassword = "";
constexpr auto configVersion = "v0.0.1";

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
      //String(FPSTR(CUSTOMHTML_BODY_INNER)) +
      IotWebConfHtmlFormatProvider::getBodyInner();
  }
};


class ConfigManager : public IotWebConf
{
public:
  ConfigManager();


private:
  void handleRoot();
  void handleDashboard();
  static bool formValidator();
  DNSServer dnsServer;
  WebServer server;
  HTTPUpdateServer httpUpdater;
  GSConfigHtmlFormatProvider gsConfigHtmlFormatProvider;

#define STRING_LEN 128
#define NUMBER_LEN 32

  char stringParamValue[STRING_LEN];
  char intParamValue[NUMBER_LEN];
  char floatParamValue[NUMBER_LEN];
  IotWebConfParameter stringParam = IotWebConfParameter("String param", "stringParam", stringParamValue, STRING_LEN);
  IotWebConfSeparator separator1 = IotWebConfSeparator();
  IotWebConfParameter intParam = IotWebConfParameter("Int param", "intParam", intParamValue, NUMBER_LEN, "number", "1..100", NULL, "min='1' max='100' step='1'");
  // -- We can add a legend to the separator
  IotWebConfSeparator separator2 = IotWebConfSeparator("Calibration factor");
  IotWebConfParameter floatParam = IotWebConfParameter("Float param", "floatParam", floatParamValue, NUMBER_LEN, "number", "e.g. 23.4", NULL, "step='0.1'");
};

