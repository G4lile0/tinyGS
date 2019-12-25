/**
 * IotWebConf.h -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *
 * Copyright (C) 2018 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef IotWebConf_h
#define IotWebConf_h

#include <IotWebConfCompatibility.h>

#ifdef ESP8266
# include <ESP8266WiFi.h>
# include <ESP8266WebServer.h>
# include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
# include <WiFi.h>
# include <WebServer.h>
#endif
#include <DNSServer.h> // -- For captive portal

// -- We might want to place the config in the EEPROM in an offset.
#define IOTWEBCONF_CONFIG_START 0

// -- Maximal length of any string used in IotWebConfig configuration (e.g.
// ssid, password).
#define IOTWEBCONF_WORD_LEN 33

// -- IotWebConf tries to connect to the local network for an amount of time
// before falling back to AP mode.
#define IOTWEBCONF_DEFAULT_WIFI_CONNECTION_TIMEOUT_MS 30000

// -- Thing will stay in AP mode for an amount of time on boot, before retrying
// to connect to a WiFi network.
#define IOTWEBCONF_DEFAULT_AP_MODE_TIMEOUT_MS 30000

// -- mDNS should allow you to connect to this device with a hostname provided
// by the device. E.g. mything.local
#define IOTWEBCONF_CONFIG_USE_MDNS

// -- Logs progress information to Serial if enabled.
#define IOTWEBCONF_DEBUG_TO_SERIAL

// -- Logs passwords to Serial if enabled.
//#define IOTWEBCONF_DEBUG_PWD_TO_SERIAL

// -- Helper define for serial debug
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
# define IOTWEBCONF_DEBUG_LINE(MSG) Serial.println(MSG)
#else
# define IOTWEBCONF_DEBUG_LINE(MSG)
#endif

// -- EEPROM config starts with a special prefix of length defined here.
#define IOTWEBCONF_CONFIG_VESION_LENGTH 4
#define IOTWEBCONF_DNS_PORT 53

// -- HTML page fragments
const char IOTWEBCONF_HTML_HEAD[] PROGMEM         = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char IOTWEBCONF_HTML_STYLE_INNER[] PROGMEM  = ".de{background-color:#ffaaaa;} .em{font-size:0.8em;color:#bb0000;padding-bottom:0px;} .c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#16A1E7;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} fieldset{border-radius:0.3rem;margin: 0px;}";
const char IOTWEBCONF_HTML_SCRIPT_INNER[] PROGMEM = "function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}";
const char IOTWEBCONF_HTML_HEAD_END[] PROGMEM     = "</head><body>";
const char IOTWEBCONF_HTML_BODY_INNER[] PROGMEM   = "<div style='text-align:left;display:inline-block;min-width:260px;'>";
const char IOTWEBCONF_HTML_FORM_START[] PROGMEM   = "<form action='' method='post'><fieldset><input type='hidden' name='iotSave' value='true'>";
const char IOTWEBCONF_HTML_FORM_PARAM[] PROGMEM   = "<div class='{s}'><label for='{i}'>{b}</label><input type='{t}' id='{i}' name='{i}' maxlength={l} placeholder='{p}' value='{v}' {c}/><div class='em'>{e}</div></div>";
const char IOTWEBCONF_HTML_FORM_END[] PROGMEM     = "</fieldset><button type='submit'>Apply</button></form>";
const char IOTWEBCONF_HTML_SAVED[] PROGMEM        = "<div>Configuration saved<br />Return to <a href='/'>home page</a>.</div>";
const char IOTWEBCONF_HTML_END[] PROGMEM          = "</div></body></html>";
const char IOTWEBCONF_HTML_UPDATE[] PROGMEM       = "<div style='padding-top:25px;'><a href='{u}'>Firmware update</a></div>";
const char IOTWEBCONF_HTML_CONFIG_VER[] PROGMEM   = "<div style='font-size: .6em;'>Firmware config version '{v}'</div>";

// -- State of the Thing
#define IOTWEBCONF_STATE_BOOT 0
#define IOTWEBCONF_STATE_NOT_CONFIGURED 1
#define IOTWEBCONF_STATE_AP_MODE 2
#define IOTWEBCONF_STATE_CONNECTING 3
#define IOTWEBCONF_STATE_ONLINE 4

// -- AP connection state
// -- No connection on AP.
#define IOTWEBCONF_AP_CONNECTION_STATE_NC 0
// -- Has connection on AP.
#define IOTWEBCONF_AP_CONNECTION_STATE_C 1
// -- All previous connection on AP was disconnected.
#define IOTWEBCONF_AP_CONNECTION_STATE_DC 2

// -- Status indicator output logical levels.
#define IOTWEBCONF_STATUS_ON LOW
#define IOTWEBCONF_STATUS_OFF HIGH

// -- User name on login.
#define IOTWEBCONF_ADMIN_USER_NAME "admin"

typedef struct IotWebConfWifiAuthInfo
{
  const char* ssid;
  const char* password;
} IotWebConfWifiAuthInfo;

/**
 *   IotWebConfParameters is a configuration item of the config portal.
 *   The parameter will have its input field on the configuration page,
 *   and the provided value will be saved to the EEPROM.
 */
class IotWebConfParameter
{
public:
  /**
   * Create a parameter for the config portal.
   *
   *   @label - Displayable label at the config portal.
   *   @id - Identifier used for HTTP queries and as configuration key. Must not contain spaces nor other special characters.
   *   @valueBuffer - Configuration value will be loaded to this buffer from the EEPROM.
   *   @length - The buffer should have a length provided here.
   *   @type (optional, default="text") - The type of the html input field.
   *       The type="password" has a special handling, as the value will be overwritten in the EEPROM
   *       only if value was provided on the config portal. Because of this logic, "password" type field with
   *       length more then IOTWEBCONF_WORD_LEN characters are not supported.
   *   @placeholder (optional) - Text appear in an empty input box.
   *   @defaultValue (optional) - Value should be pre-filled if none was specified before.
   *   @customHtml (optional) - The text of this parameter will be added into the HTML INPUT field.
   */
  IotWebConfParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* type = "text", const char* placeholder = NULL,
    const char* defaultValue = NULL, const char* customHtml = NULL,
    boolean visible = true);

  /**
   * Same as normal constructor, but config portal does not render a default
   * input field for the item, instead uses the customHtml provided. Note the
   * @type parameter description above!
   */
  IotWebConfParameter(
      const char* id, char* valueBuffer, int length, const char* customHtml,
      const char* type = "text");

  /**
   * For internal use only.
   */
  IotWebConfParameter();

  const char* label;
  char* valueBuffer;
  const char* type;
  const char* placeholder;
  const char* defaultValue;
  const char* customHtml;
  boolean visible;
  const char* errorMessage;

  // -- For internal use only
  IotWebConfParameter* _nextParameter = NULL;

  const char* getId() { return this->_id; }
  int getLength() { return this->_length; }

private:
  const char* _id = 0;
  int _length;
};

/**
 * A separator for separating field sets.
 */
class IotWebConfSeparator : public IotWebConfParameter
{
public:
  IotWebConfSeparator();

  /**
   * Create a seperator with a label (legend tag)
   */
  IotWebConfSeparator(const char* label);
};

/**
 * Class for providing HTML format segments.
 */
class IotWebConfHtmlFormatProvider
{
public:
  virtual String getHead() { return FPSTR(IOTWEBCONF_HTML_HEAD); }
  virtual String getStyle() { return "<style>" + getStyleInner() + "</style>"; }
  virtual String getScript() { return "<script>" + getScriptInner() + "</script>"; }
  virtual String getHeadExtension() { return ""; }
  virtual String getHeadEnd() { return String(FPSTR(IOTWEBCONF_HTML_HEAD_END)) + getBodyInner(); }
  virtual String getFormStart() { return FPSTR(IOTWEBCONF_HTML_FORM_START); }
  virtual String getFormParam(const char* type) { return FPSTR(IOTWEBCONF_HTML_FORM_PARAM); }
  virtual String getFormEnd() { return FPSTR(IOTWEBCONF_HTML_FORM_END); }
  virtual String getFormSaved() { return FPSTR(IOTWEBCONF_HTML_SAVED); }
  virtual String getEnd() { return FPSTR(IOTWEBCONF_HTML_END); }
  virtual String getUpdate() { return FPSTR(IOTWEBCONF_HTML_UPDATE); }
  virtual String getConfigVer() { return FPSTR(IOTWEBCONF_HTML_CONFIG_VER); }
protected:
  virtual String getStyleInner() { return FPSTR(IOTWEBCONF_HTML_STYLE_INNER); }
  virtual String getScriptInner() { return FPSTR(IOTWEBCONF_HTML_SCRIPT_INNER); }
  virtual String getBodyInner() { return FPSTR(IOTWEBCONF_HTML_BODY_INNER); }
};

/**
 * Main class of the module.
 */
class IotWebConf
{
public:
  /**
   * Create a new configuration handler.
   *   @thingName - Initial value for the thing name. Used in many places like AP name, can be changed by the user.
   *   @dnsServer - A created DNSServer, that can be configured for captive portal.
   *   @server - A created web server. Will be started upon connection success.
   *   @initialApPassword - Initial value for AP mode. Can be changed by the user.
   *   @configVersion - When the software is updated and the configuration is changing, this key should also be changed,
   *     so that the config portal will force the user to reenter all the configuration values.
   */
  IotWebConf(
      const char* thingName, DNSServer* dnsServer, WebServer* server,
      const char* initialApPassword, const char* configVersion = "init");

  /**
   * Provide an Arduino pin here, that has a button connected to it with the other end of the pin is connected to GND.
   * The button pin is queried at for input on boot time (init time).
   * If the button was pressed, the thing will enter AP mode with the initial password.
   * Must be called before init()!
   *   @configPin - An Arduino pin. Will be configured as INPUT_PULLUP!
   */
  void setConfigPin(int configPin);

  /**
   * Provide an Arduino pin for status indicator (LOW = on). Blink codes:
   *   - Rapid blinks - The thing is in AP mode with default password.
   *   - Rapid blinks, but mostly on - AP mode, waiting for configuration changes.
   *   - Normal blinks - Connecting to WiFi.
   *   - Mostly off with rare rapid blinks - WiFi is connected performing normal operation.
   * User can also apply custom blinks. See blink() method!
   * Must be called before init()!
   *   @statusPin - An Arduino pin. Will be configured as OUTPUT!
   */
  void setStatusPin(int statusPin);

  /**
   * Add an UpdateServer instance to the system. The firmware update link will appear on the config portal.
   * The UpdateServer will be added to the WebServer with the path provided here (or with "firmware",
   * if none was provided).
   * Login user will be IOTWEBCONF_ADMIN_USER_NAME, password is the password provided in the config portal.
   * Should be called before init()!
   *   @updateServer - An uninitialized UpdateServer instance.
   *   @updatePath - (Optional) The path to set up the UpdateServer with. Will be also used in the config portal.
   */
  void setupUpdateServer(
      HTTPUpdateServer* updateServer, const char* updatePath = "/firmware");

  /**
   * Start up the IotWebConf module.
   * Loads all configuration from the EEPROM, and initialize the system.
   * Will return false, if no configuration (with specified config version) was found in the EEPROM.
   */
  boolean init();

  /**
   * IotWebConf is a non-blocking, state controlled system. Therefor it should be
   * regularly triggered from the user code.
   * So call this method any time you can.
   */
  void doLoop();

  /**
   * Each WebServer URL handler method should start with calling this method.
   * If this method return true, the request was already served by it.
   */
  boolean handleCaptivePortal();

  /**
   * Config URL web request handler. Call this method to handle config request.
   */
  void handleConfig();

  /**
   * URL-not-found web request handler. Used for handling captive portal request.
   */
  void handleNotFound();

  /**
   * Specify a callback method, that will be called upon WiFi connection success.
   * Should be called before init()!
   */
  void setWifiConnectionCallback(std::function<void()> func);

  /**
   * Specify a callback method, that will be called when settings have been changed.
   * Should be called before init()!
   */
  void setConfigSavedCallback(std::function<void()> func);

  /**
   * Specify a callback method, that will be called when form validation is required.
   * If the method will return false, the configuration will not be saved.
   * Should be called before init()!
   */
  void setFormValidator(std::function<boolean()> func);

  /**
   * Specify your custom Access Point connection handler. Please use IotWebConf::connectAp() as
   * reference when implementing your custom solution.
   */
  void setApConnectionHandler(
      std::function<boolean(const char* apName, const char* password)> func)
  {
    _apConnectionHandler = func;
  }

  /**
   * Specify your custom WiFi connection handler. Please use IotWebConf::connectWifi() as
   * reference when implementing your custom solution.
   */
  void setWifiConnectionHandler(
      std::function<void(const char* ssid, const char* password)> func)
  {
    _wifiConnectionHandler = func;
  }

  /**
   * With this method you can specify your custom WiFi timeout handler.
   * This hander can manage what should happen, when WiFi connection timed out.
   * By default the handler implementation returns with NULL, as seen on reference implementation
   * IotWebConf::handleConnectWifiFailure(). This means we need to fall back to AP mode.
   * If it method returns with a (new) WiFi settings, it is used as a next try.
   * Note, that this feature is provided because of a possible future option of providing multiply
   * WiFi settings.
   */
  void setWifiConnectionFailedHandler( std::function<IotWebConfWifiAuthInfo*()> func )
  {
    _wifiConnectionFailureHandler = func;
  }

  /**
   * Add a custom parameter, that will be handled by the IotWebConf module.
   * The parameter will be saved to/loaded from EEPROM automatically,
   * and will appear on the config portal.
   * Will return false, if adding was not successful.
   * Must be called before init()!
   */
  bool addParameter(IotWebConfParameter* parameter);

  /**
   * Getter for the actually configured thing name.
   */
  char* getThingName();

  /**
   * Use this delay, to prevent blocking IotWebConf.
   */
  void delay(unsigned long millis);

  /**
   * IotWebConf tries to connect to the local network for an amount of time before falling back to AP mode.
   * The default amount can be updated with this setter.
   * Should be called before init()!
   */
  void setWifiConnectionTimeoutMs(unsigned long millis);

  /**
   * Interrupts internal blinking cycle and applies new values for
   * blinking the status LED (if one configured with setStatusPin() prior init()
   * ).
   *   @repeatMs - Defines the the period of one on-off cycle in milliseconds.
   *   @dutyCyclePercent - LED on/off percent. 100 means always on, 0 means
   * always off. When called with repeatMs = 0, then internal blink cycle will
   * be continued.
   */
  void blink(unsigned long repeatMs, byte dutyCyclePercent);

  /**
   * Similar to blink, but here we define exact on and off times for more
   * precise timings.
   *   @onMs - Milliseconds for the LED tudned on.
   *   @offMs -  Milliseconds for the LED tudned off.
   */
  void fineBlink(unsigned long onMs, unsigned long offMs);

  /**
   * Stop custom blinking defined by blink() or fineBlink() and continues with
   * the internal blink cycle.
   */
  void stopCustomBlink();

  /**
   * Return the current state, that will be a value from the IOTWEBCONF_STATE_* constants.
   */
  byte getState() { return this->_state; };

  /**
   * This method can be used to set the AP timeout directly without modifying the apTimeoutParameter.
   * Note, that apTimeoutMs value will be reset to the value of apTimeoutParameter on init and on config save.
   */
  void setApTimeoutMs(unsigned long apTimeoutMs)
  {
    this->_apTimeoutMs = apTimeoutMs;
  };

  /**
   * Returns the actual value of the AP timeout in use.
   */
  unsigned long getApTimeoutMs() { return this->_apTimeoutMs; };

  /**
   * Resets the authentication credentials for WiFi connection to the configured one.
   * With the return value of setWifiConnectionFailedHandler() one can provide alternative connection settings,
   * that can be reset with resetWifiAuthInfo().
   */
  void resetWifiAuthInfo()
  {
    _wifiAuthInfo = {this->_wifiSsid, this->_wifiPassword};
  };

  /**
   * By default IotWebConf starts up in AP mode. Calling this method before the init will force IotWebConf
   * to connect immediatelly to the configured WiFi network.
   * Note, this method only takes effect, when WiFi mode is enabled, thus when a valid WiFi connection is
   * set up, and AP mode is not forced by ConfigPin (see setConfigPin() for details).
   */
  void skipApStartup() { this->_skipApStartup = true; }

  /**
   * Get internal parameters, for manual handling.
   * Normally you don't need to access these parameters directly.
   * Note, that changing valueBuffer of these parameters should be followed by configSave()!
   */
  IotWebConfParameter* getThingNameParameter()
  {
    return &this->_thingNameParameter;
  };
  IotWebConfParameter* getApPasswordParameter()
  {
    return &this->_apPasswordParameter;
  };
  IotWebConfParameter* getWifiSsidParameter()
  {
    return &this->_wifiSsidParameter;
  };
  IotWebConfParameter* getWifiPasswordParameter()
  {
    return &this->_wifiPasswordParameter;
  };
  IotWebConfParameter* getApTimeoutParameter()
  {
    return &this->_apTimeoutParameter;
  };

  /**
   * If config parameters are modified directly, the new values can be saved by this method.
   * Note, that init() must pretend configSave()!
   * Also note, that configSave writes to EEPROM, and EEPROM can be written only some thousand times
   *  in the lifetime of an ESP8266 module.
   */
  void configSave();

  /**
   * With this method you can override the default HTML format provider to
   * provide custom HTML segments.
   */
  void
  setHtmlFormatProvider(IotWebConfHtmlFormatProvider* customHtmlFormatProvider)
  {
    this->htmlFormatProvider = customHtmlFormatProvider;
  }
  IotWebConfHtmlFormatProvider* getHtmlFormatProvider()
  {
    return this->htmlFormatProvider;
  }

private:
  const char* _initialApPassword = NULL;
  const char* _configVersion;
  DNSServer* _dnsServer;
  WebServer* _server;
  HTTPUpdateServer* _updateServer = NULL;
  int _configPin = -1;
  int _statusPin = -1;
  const char* _updatePath = NULL;
  boolean _forceDefaultPassword = false;
  boolean _skipApStartup = false;
  IotWebConfParameter* _firstParameter = NULL;
  IotWebConfParameter _thingNameParameter;
  IotWebConfParameter _apPasswordParameter;
  IotWebConfParameter _wifiSsidParameter;
  IotWebConfParameter _wifiPasswordParameter;
  IotWebConfParameter _apTimeoutParameter;
  char _thingName[IOTWEBCONF_WORD_LEN];
  char _apPassword[IOTWEBCONF_WORD_LEN];
  char _wifiSsid[IOTWEBCONF_WORD_LEN];
  char _wifiPassword[IOTWEBCONF_WORD_LEN];
  char _apTimeoutStr[IOTWEBCONF_WORD_LEN];
  unsigned long _apTimeoutMs = IOTWEBCONF_DEFAULT_AP_MODE_TIMEOUT_MS;
  unsigned long _wifiConnectionTimeoutMs =
      IOTWEBCONF_DEFAULT_WIFI_CONNECTION_TIMEOUT_MS;
  byte _state = IOTWEBCONF_STATE_BOOT;
  unsigned long _apStartTimeMs = 0;
  byte _apConnectionStatus = IOTWEBCONF_AP_CONNECTION_STATE_NC;
  std::function<void()> _wifiConnectionCallback = NULL;
  std::function<void()> _configSavedCallback = NULL;
  std::function<boolean()> _formValidator = NULL;
  std::function<void(const char*, const char*)> _apConnectionHandler =
      &(IotWebConf::connectAp);
  std::function<void(const char*, const char*)> _wifiConnectionHandler =
      &(IotWebConf::connectWifi);
  std::function<IotWebConfWifiAuthInfo*()> _wifiConnectionFailureHandler =
      &(IotWebConf::handleConnectWifiFailure);
  unsigned long _internalBlinkOnMs = 500;
  unsigned long _internalBlinkOffMs = 500;
  unsigned long _blinkOnMs = 500;
  unsigned long _blinkOffMs = 500;
  byte _blinkState = IOTWEBCONF_STATUS_ON;
  unsigned long _lastBlinkTime = 0;
  unsigned long _wifiConnectionStart = 0;
  IotWebConfWifiAuthInfo _wifiAuthInfo = {_wifiSsid, _wifiPassword};
  IotWebConfHtmlFormatProvider htmlFormatProviderInstance;
  IotWebConfHtmlFormatProvider* htmlFormatProvider = &htmlFormatProviderInstance;

  void configInit();
  boolean configLoad();
  boolean configTestVersion();
  void configSaveConfigVersion();
  void readEepromValue(int start, char* valueBuffer, int length);
  void writeEepromValue(int start, char* valueBuffer, int length);

  void readParamValue(const char* paramName, char* target, unsigned int len);
  boolean validateForm();

  void changeState(byte newState);
  void stateChanged(byte oldState, byte newState);
  boolean isWifiModePossible()
  {
    return this->_forceDefaultPassword || (this->_apPassword[0] == '\0');
  }
  boolean isIp(String str);
  String toStringIp(IPAddress ip);
  void doBlink();
  void blinkInternal(unsigned long repeatMs, byte dutyCyclePercent);

  void checkApTimeout();
  void checkConnection();
  boolean checkWifiConnection();
  void setupAp();
  void stopAp();

  static boolean connectAp(const char* apName, const char* password);
  static void connectWifi(const char* ssid, const char* password);
  static IotWebConfWifiAuthInfo* handleConnectWifiFailure();
};

#endif
