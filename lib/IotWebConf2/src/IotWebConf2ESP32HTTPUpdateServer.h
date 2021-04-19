/**
 * IotWebConfESP32HTTPUpdateServer.h -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 *
 * Notes on IotWebConfESP32HTTPUpdateServer:
 * ESP32 doesn't implement a HTTPUpdateServer. However it seams, that to code
 * from ESP8266 covers nearly all the same functionality.
 * So we need to implement our own HTTPUpdateServer for ESP32, and code is
 * reused from
 * https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPUpdateServer/src/
 * version: 41de43a26381d7c9d29ce879dd5d7c027528371b
 */
#ifdef ESP32

#ifndef __HTTP_UPDATE_SERVER_H
#define __HTTP_UPDATE_SERVER_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <StreamString.h>
#include <Update.h>

#define emptyString F("")

class WebServer;

class HTTPUpdateServer
{
  public:
    HTTPUpdateServer(bool serial_debug=false)
    {
        _serial_output = serial_debug;
        _server = NULL;
        _username = emptyString;
        _password = emptyString;
        _authenticated = false;
    }


    void setup(WebServer *server)
    {
      setup(server, emptyString, emptyString);
    }

    void setup(WebServer *server, const String& path)
    {
      setup(server, path, emptyString, emptyString);
    }

    void setup(WebServer *server, const String& username, const String& password)
    {
      setup(server, "/update", username, password);
    }

    void setup(WebServer *server, const String& path, const String& username, const String& password)
    {
      _server = server;
      _username = username;
      _password = password;

      // handler for the /update form page
      _server->on(path.c_str(), HTTP_GET, [&](){
          if(_username != emptyString && _password != emptyString && !_server->authenticate(_username.c_str(), _password.c_str()))
              return _server->requestAuthentication();
          _server->send_P(200, PSTR("text/html"), serverIndex);
      });

      // handler for the /update form POST (once file upload finishes)
      _server->on(path.c_str(), HTTP_POST, [&](){
          if(!_authenticated)
              return _server->requestAuthentication();
          if (Update.hasError()) {
              _server->send(200, F("text/html"), String(F("Update error: ")) + _updaterError);
          } else {
              _server->client().setNoDelay(true);
              _server->send_P(200, PSTR("text/html"), successResponse);
              delay(100);
              _server->client().stop();
              ESP.restart();
          }
      },[&](){
          // handler for the file upload, get's the sketch bytes, and writes
          // them through the Update object
          HTTPUpload& upload = _server->upload();

          if(upload.status == UPLOAD_FILE_START){
              _updaterError = String();
              if (_serial_output)
                  Serial.setDebugOutput(true);

              _authenticated = (_username == emptyString || _password == emptyString || _server->authenticate(_username.c_str(), _password.c_str()));
              if(!_authenticated){
                  if (_serial_output)
                      Serial.printf("Unauthenticated Update\n");
                  return;
              }

  ///        WiFiUDP::stopAll();
              if (_serial_output)
                  Serial.printf("Update: %s\n", upload.filename.c_str());
  ///        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  ///        if(!Update.begin(maxSketchSpace)){//start with max available size
              if(!Update.begin(UPDATE_SIZE_UNKNOWN)){//start with max available size
                  _setUpdaterError();
              }
          } else if(_authenticated && upload.status == UPLOAD_FILE_WRITE && !_updaterError.length()){
              if (_serial_output) Serial.printf(".");
              if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
                  _setUpdaterError();
              }
          } else if(_authenticated && upload.status == UPLOAD_FILE_END && !_updaterError.length()){
              if(Update.end(true)){ //true to set the size to the current progress
                  if (_serial_output) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
              } else {
                  _setUpdaterError();
              }
              if (_serial_output) Serial.setDebugOutput(false);
          } else if(_authenticated && upload.status == UPLOAD_FILE_ABORTED){
              Update.end();
              if (_serial_output) Serial.println("Update was aborted");
          }
          delay(0);
      });
    }

    void updateCredentials(const String& username, const String& password)
    {
      _username = username;
      _password = password;
    }

  protected:
    void _setUpdaterError()
    {
        if (_serial_output) Update.printError(Serial);
        StreamString str;
        Update.printError(str);
        _updaterError = str.c_str();
    }

  private:
    bool _serial_output;
    WebServer *_server;
    String _username;
    String _password;
    bool _authenticated;
    String _updaterError;
    const char* serverIndex PROGMEM =
R"(<html><body><form method='POST' action='' enctype='multipart/form-data'>
                  <input type='file' name='update'>
                  <input type='submit' value='Update'>
               </form>
         </body></html>)";
  const char* successResponse PROGMEM =
"<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...\n";
};

/////////////////////////////////////////////////////////////////////////////////

#endif

#endif

