/**
 * IotWebConf2OptionalGroup.h -- IotWebConf2 is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf2
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef IotWebConf2OptionalGroup_h
#define IotWebConf2OptionalGroup_h

#include "IotWebConf2.h" // For HtmlFormatProvider ... TODO: should be reorganized
#include "IotWebConf2Parameter.h"

const char IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_JAVASCRIPT[] PROGMEM =
  "    function show(id) { var x=document.getElementById(id); x.classList.remove('hide'); }\n"
  "    function hide(id) { var x=document.getElementById(id); x.classList.add('hide'); }\n"
  "    function val(id) { var x=document.getElementById(id); return x.value; }\n"
  "    function setVal(id, val) { var x=document.getElementById(id); x.value = val; }\n"
  "    function showFs(id) {\n"
  "      show(id); hide(id + 'b'); setVal(id + 'v', 'active'); var n=document.getElementById(id + 'next');\n"
  "      if (n) { var nId = n.value; if (val(nId + 'v') == 'inactive') { show(nId + 'b'); }}\n"
  "    }\n"
  "    function hideFs(id) {\n"
  "      hide(id); show(id + 'b'); setVal(id + 'v', 'inactive'); var n=document.getElementById(id + 'next');\n"
  "      if (n) { var nId = n.value; if (val(nId + 'v') == 'inactive') { hide(nId + 'b'); }}\n"
  "    }\n";
const char IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_CSS[] PROGMEM =
  ".hide{display: none;}\n";
const char IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_START[] PROGMEM =
  "<button id='{i}b' class='{cb}' onclick=\"showFs('{i}'); return false;\">+ {b}</button>\n"
  "<fieldset id='{i}' class='{cf}'><legend>{b}</legend>\n"
  "<button onclick=\"hideFs('{i}'); return false;\">Remove this set</button>\n"
  "<input id='{i}v' name='{i}v' type='hidden' value='{v}'/>\n"
  "\n";
const char IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_END[] PROGMEM =
  "</fieldset>\n";
const char IOTWEBCONF_HTML_FORM_CHAINED_GROUP_NEXTID[] PROGMEM =
  "<input type='hidden' id='{i}next' value='{in}'/>\n";

namespace iotwebconf2
{

class OptionalGroupHtmlFormatProvider : public HtmlFormatProvider
{
protected:
  String getScriptInner() override
  {
    return
      HtmlFormatProvider::getScriptInner() +
      String(FPSTR(IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_JAVASCRIPT));
  }
  String getStyleInner() override
  {
    return
      HtmlFormatProvider::getStyleInner() +
      String(FPSTR(IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_CSS));
  }
};

/**
 * With OptionalParameterGroup buttons will appear in the GUI,
 * to show and hide this specific group of parameters. The idea
 * behind this feature to add/remove optional parameter set
 * in the config portal.
 */
class OptionalParameterGroup : public ParameterGroup
{
public:
  OptionalParameterGroup(const char* id, const char* label, bool defaultActive);
  bool isActive() { return this->_active; }
  void setActive(bool active) { this->_active = active; }

protected:
  int getStorageSize() override;
  void applyDefaultValue() override;
  void storeValue(std::function<void(
    SerializationData* serializationData)> doStore) override;
  void loadValue(std::function<void(
    SerializationData* serializationData)> doLoad) override;
  void renderHtml(bool dataArrived, WebRequestWrapper* webRequestWrapper) override;
  virtual String getStartTemplate() { return FPSTR(IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_START); };
  virtual String getEndTemplate() { return FPSTR(IOTWEBCONF_HTML_FORM_OPTIONAL_GROUP_END); };
  void update(WebRequestWrapper* webRequestWrapper) override;
  void debugTo(Stream* out) override;

private:
  bool _defaultActive;
  bool _active;
};

class ChainedParameterGroup;

class ChainedParameterGroup : public OptionalParameterGroup
{
public:
  ChainedParameterGroup(const char* id, const char* label, bool defaultActive = false) :
    OptionalParameterGroup(id, label, defaultActive) { };
  void setNext(ChainedParameterGroup* nextGroup) { this->_nextGroup = nextGroup; nextGroup->_prevGroup = this; };
  ChainedParameterGroup* getNext() { return this->_nextGroup; };

protected:
  virtual String getStartTemplate() override;
  virtual String getEndTemplate() override;

protected:
  ChainedParameterGroup* _prevGroup = NULL;
  ChainedParameterGroup* _nextGroup = NULL;
};

}

#endif