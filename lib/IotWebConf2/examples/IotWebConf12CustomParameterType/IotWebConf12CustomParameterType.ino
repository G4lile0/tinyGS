/**
 * IotWebConf12CustomParameterType.ino -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf 
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
 * Example: Custom parameter type
 * Description:
 *   In this example we will inherit the Parameter class to have
 *   a custom visualization effect inside an HTML form field.
 *   To be more specific we will create a range-slider, and we also
 *   have a numeric representation of the selected value above
 *   the slider.
 *   In our example the input-range can be rendered just as TextParameter
 *   does the render, the only major difference is, that we need to
 *   use a different template to start with.
 *   We also need to modify the HTML templates to inject some javascript
 *   codes.
 * 
 * Hardware setup for this example:
 *   - An LED is attached to LED_BUILTIN pin with setup On=LOW.
 *   - [Optional] A push button is attached to pin D2, the other leg of the
 *     button should be attached to GND.
 */

#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "testThing";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "smrtTHNG8266";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "dem3"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN D2

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Maximal length the input-range attributes can have.
#define RANGE_ATTR_LENGTH 60

const char CUSTOMHTML_SCRIPT_INNER[] PROGMEM = "\n\
    function sliderCh(id)\n\
    {\n\
      var x=document.getElementById(id);\n\
      var s=document.getElementById(id + 'Val');\n\
      s.innerHTML = x.value;\n\
    }\n";

const char IOTWEBCONF_HTML_FORM_RANGE_PARAM[] PROGMEM =
  "<div class='{s}'><label for='{i}'>{b}</label> (<span id='{i}Val'>{v}</span>)"
  "<input type='{t}' id='{i}' "
  "name='{i}' maxlength={l} placeholder='{p}' value='{v}' {c}/>"
  "<div class='em'>{e}</div></div>\n";
  
// -- Our custom class declaration. You should move it to a .h fine in your project.
class RangeWithValueParameter : public iotwebconf::NumberParameter
{
public:
  RangeWithValueParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* defaultValue,
    int min, int max, int step) : iotwebconf::NumberParameter(
      label, id, valueBuffer, length, defaultValue)
    {
      snprintf(
        this->_rangeAttr, RANGE_ATTR_LENGTH,
        "min='%d' max='%d' step='%d' oninput='sliderCh(this.id)'",
        min, max, step);

      this->customHtml = this->_rangeAttr;
    };
protected:
  // Overrides
  virtual String renderHtml(
    bool dataArrived, bool hasValueFromPost, String valueFromPost) override
  {
    return TextParameter::renderHtml("range", hasValueFromPost, valueFromPost);
  };
  virtual String getHtmlTemplate()
  {
    return FPSTR(IOTWEBCONF_HTML_FORM_RANGE_PARAM);
  };

private:
  char _rangeAttr[RANGE_ATTR_LENGTH];
};

// -- We need to create our custom HtmlFormatProvider to add some javasripts.
class CustomHtmlFormatProvider : public iotwebconf::HtmlFormatProvider
{
protected:
  String getScriptInner() override
  {
    return
      HtmlFormatProvider::getScriptInner() +
      String(FPSTR(CUSTOMHTML_SCRIPT_INNER));
  }
};

// -- Method declarations.
void handleRoot();

DNSServer dnsServer;
WebServer server(80);

char rangeParamValue[NUMBER_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
RangeWithValueParameter rangeParam = RangeWithValueParameter("Select from range", "rangeParam", rangeParamValue, NUMBER_LEN,
 "20", 1, 100, 1);

// -- An instance must be created from the class defined above.
CustomHtmlFormatProvider customHtmlFormatProvider;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.setHtmlFormatProvider(&customHtmlFormatProvider);
  iotWebConf.addSystemParameter(&rangeParam);

  // -- Initializing the configuration.
  iotWebConf.init();

  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  Serial.println("Ready.");
}

void loop() 
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 12 Custom Parameter Type</title></head><body><div>Status page of ";
  s += iotWebConf.getThingName();
  s += ".</div>";
  s += "<ul>";
  s += "<li>Selected range value: ";
  s += atoi(rangeParamValue);
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}
