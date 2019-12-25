#include "ConfigManager.h"

ConfigManager::ConfigManager()
: IotWebConf(thingName, &dnsServer, &server, initialApPassword, configVersion)
, server(80)
{
  server.on(ROOT_URL, [this]{ handleRoot(); });
  server.on(CONFIG_URL, [this]{ handleConfig(); });
  server.on(DASHBOARD_URL, [this]{ handleDashboard(); });
  setStatusPin(0);
  setConfigPin(LED_BUILTIN);
  setupUpdateServer(&httpUpdater);
  setHtmlFormatProvider(&gsConfigHtmlFormatProvider);
  formValidatorStd = std::bind(&ConfigManager::formValidator, this);
  setFormValidator(formValidatorStd);

  addParameter(&stringParam);
  addParameter(&separator1);
  addParameter(&intParam);
  addParameter(&separator2);
  addParameter(&floatParam);
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


bool ConfigManager::formValidator()
{
  Serial.println("Validating form.");
  boolean valid = true;

  int l = server.arg(stringParam.getId()).length();
  if (l < 3)
  {
    stringParam.errorMessage = "Please provide at least 3 characters for this test!";
    valid = false;
  }

  return true;
}