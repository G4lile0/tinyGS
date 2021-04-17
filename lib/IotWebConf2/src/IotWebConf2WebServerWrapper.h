/**
 * IotWebConf2WebServerWrapper.h -- IotWebConf2 is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf2
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef WebServerWrapper_h
#define WebServerWrapper_h

#include <Arduino.h>
#include <IPAddress.h>

namespace iotwebconf2
{

class WebRequestWrapper
{
public:
  virtual const String hostHeader() const;
  virtual IPAddress localIP();
  virtual uint16_t localPort();
  virtual const String uri() const;
  virtual bool authenticate(const char * username, const char * password);
  virtual void requestAuthentication();
  virtual bool hasArg(const String& name);
  virtual String arg(const String name);
  virtual void sendHeader(const String& name, const String& value, bool first = false);
  virtual void setContentLength(const size_t contentLength);
  virtual void send(int code, const char* content_type = NULL, const String& content = String(""));
  virtual void sendContent(const String& content);
  virtual void stop();
};

class WebServerWrapper
{
public:
  virtual void handleClient();
  virtual void begin();
};

} // end namespace
#endif