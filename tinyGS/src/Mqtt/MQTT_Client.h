/*
  MQTTClient.h - MQTT connection class
  
  Copyright (C) 2020 -2021 @G4lile0, @gmag12 and @dev_4m1g0

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

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#define SECURE_MQTT // Comment this line if you are not using MQTT over SSL
#define LOG_TAG "MQTT"

#include "../ConfigManager/ConfigManager.h"
#include "../Status.h"
#define MQTT_MAX_PACKET_SIZE 1000
#include <PubSubClient.h>
#ifdef SECURE_MQTT
#include <WiFiClientSecure.h>

static const char DSTroot_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFJDCCBAygAwIBAgISA4sFj+FWbCixGLUvw3IzQ77/MA0GCSqGSIb3DQEBCwUA
MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD
EwJSMzAeFw0yMTAxMTcyMDQ4MDNaFw0yMTA0MTcyMDQ4MDNaMBoxGDAWBgNVBAMT
D25vZGUudGlueWdzLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
ALRIqgk7C32ALt7gyqPIxzV6uiv6qLtvh+i1HZuunz2NcGyD/M1/qC3n+2/8MZ4c
j9s1WyoPs5+aXU+59Kb7yIKNV8tHPiSI2HhI04EbuGOPxGNmkXzzSHeDuatnKF0Q
Zv3cAuufNiapW3tOF9vk4NtUSemwxF7PwpYv8YEf/WnIIi44hg3el73pDosaCGGi
StgWTuTBEWgJ9IgxhWfoy3Ow+7AbMapU/MvtNaQVTUR5MclSHEXIoF4sSqk81eoX
7CMgWiJR1Gaf+CYUEqGLXrNZ8uYmpOqR2b1eZ7zInSKtu4kmyZDZyfe7yenLIzS6
QqoFZE+sHByxqFw4YCKZvP0CAwEAAaOCAkowggJGMA4GA1UdDwEB/wQEAwIFoDAd
BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNV
HQ4EFgQUXwjbt4+v50XerBQWpszvEEZnGhYwHwYDVR0jBBgwFoAUFC6zF7dYVsuu
UAlA5h+vnYsUwsYwVQYIKwYBBQUHAQEESTBHMCEGCCsGAQUFBzABhhVodHRwOi8v
cjMuby5sZW5jci5vcmcwIgYIKwYBBQUHMAKGFmh0dHA6Ly9yMy5pLmxlbmNyLm9y
Zy8wGgYDVR0RBBMwEYIPbm9kZS50aW55Z3MuY29tMEwGA1UdIARFMEMwCAYGZ4EM
AQIBMDcGCysGAQQBgt8TAQEBMCgwJgYIKwYBBQUHAgEWGmh0dHA6Ly9jcHMubGV0
c2VuY3J5cHQub3JnMIIBBAYKKwYBBAHWeQIEAgSB9QSB8gDwAHcAXNxDkv7mq0VE
sV6a1FbmEDf71fpH3KFzlLJe5vbHDsoAAAF3ElHBxAAABAMASDBGAiEAyLha1SRA
ZQMBdS73r/gm0N6agSCvUk8QNU/QA6ikO2YCIQDN9bhnl4yQm6MApt8+Y0711/45
uQglW9eKpp3hmnUhUwB1APZclC/RdzAiFFQYCDCUVo7jTRMZM7/fDC8gC8xO8WTj
AAABdxJRwcAAAAQDAEYwRAIgRunWbjzKLWXlHB9wMA4znUSPVLkwLuGq84Yh0mnG
OHcCIG/iF8+r8JZIDI34hlfz4jPJ91XuakQXprzTX5yoA67NMA0GCSqGSIb3DQEB
CwUAA4IBAQCNyN6zIElO860JiE+AKkgNUJjLsqAx3y+o4KkrWL6Y/2CgYHZKpPhv
4t/9AG0w0cI0vjXtnBG/LaCb7g7U7SG3K/pfs2MTNyyvKyTuasTgiLapEju2+XBg
Hec431YtnQwk1ALKapXEieQ1lA1AZLogB78Adws8QcI/laVhU7XFHPJkHW0YiQOU
j7Jj8Oz9XfR+MPUzpoENalZdQ8QkqwkqEkchJebPGiGWGyFwf9NIEU+RgiKs1COM
YX01a5MdJdZox2B2nPkwVrUdaUFois/WKMjG9Io3p3pjHeIS+/6ZxGXdWelvp9JN
1hmQ/Xz7PYCYFe4fTdobcjdWQ0fzQ2q7
-----END CERTIFICATE-----
)EOF";
#else
#include <WiFiClient.h>
#endif

extern Status status;

class MQTT_Client : public PubSubClient {
public:
  static MQTT_Client& getInstance()
  {
    static MQTT_Client instance; 
    return instance;
  }
  void begin();
  void loop();
  void sendWelcome();
  void sendSystemInfo();
  void sendPong();
  void sendMessage(char* frame, size_t respLen);
  void sendRawPacket(String packet);
  void manageMQTTData(char *topic, uint8_t *payload, unsigned int length);
  void sendStatus();

protected:
#ifdef SECURE_MQTT
  WiFiClientSecure espClient;
#else
  WiFiClient espClient;
#endif
  void reconnect();

private:
  MQTT_Client();
  String buildTopic(const char * topic);
  void subscribeToAll();
  void manageSatPosOled(char* payload, size_t payload_len);

  unsigned long lastPing = 0;
  unsigned long lastConnectionAtempt = 0;
  uint8_t connectionAtempts = 0;

  const unsigned long pingInterval = 1 * 60 * 1000;
  const unsigned long reconnectionInterval = 5 * 1000;
  uint16_t connectionTimeout = 5 * 60 * 1000 / reconnectionInterval;

  const char* topicStart PROGMEM = "tinygs";
  const char* topicWelcome PROGMEM = "welcome";
  const char* topicStatus PROGMEM = "status";
  const char* topicSysInfo PROGMEM = "sys_info";
  const char* topicPong PROGMEM= "pong";
  const char* topicMsg PROGMEM= "msg";
  const char* topicMiniTTN PROGMEM= "miniTTN";
  const char* topicData PROGMEM= "data/#";
  const char* topicPing PROGMEM= "ping";
  const char* topicRawPacket PROGMEM= "raw_packet";
  const char* topicSendStatus PROGMEM= "gs_status";
  
  const char* topicRemote PROGMEM= "data/remote/";
  const char* topicGlobalRemote PROGMEM= "tinygs/global/remote/";
  const char* topicRemoteReset PROGMEM= "reset";
  const char* topicRemotePing PROGMEM= "ping";
  const char* topicRemoteFreq PROGMEM= "freq";
  const char* topicRemoteBw PROGMEM= "bw";
  const char* topicRemoteSf PROGMEM= "sf";
  const char* topicRemoteCr PROGMEM= "cr";
  const char* topicRemoteCrc PROGMEM= "crc";
  const char* topicRemoteLsw PROGMEM= "lsw";
  const char* topicRemoteFldro PROGMEM= "fldro";
  const char* topicRemoteAldro PROGMEM= "aldro";
  const char* topicRemotePl PROGMEM= "pl";
  const char* topicRemoteBl PROGMEM= "begin_lora";
  const char* topicRemoteFs PROGMEM= "begin_fsk";
  const char* topicRemoteBr PROGMEM= "br";
  const char* topicRemoteFd PROGMEM= "Fd";
  const char* topicRemoteFbw PROGMEM= "fbw";
  const char* topicRemoteFsw PROGMEM= "fsw";
  const char* topicRemoteFook PROGMEM= "fok";
  const char* topicRemoteLocalFrame PROGMEM= "frame_l";
  const char* topicRemoteLocalFrame1 PROGMEM= "frame_l1";
  const char* topicRemoteLocalFrame2 PROGMEM= "frame_l2";
  const char* topicRemoteSat PROGMEM= "sat";
  const char* topicRemoteStatus PROGMEM= "status";
  const char* topicRemoteTest PROGMEM= "test";
  const char* topicRemoteremoteTune PROGMEM= "remoteTune";
  const char* topicRemotetelemetry3rd PROGMEM= "telemetry3rd";
  const char* topicRemoteJSON PROGMEM= "json";
  
    // GOD MODE  With great power comes great responsibility!
  const char* topicSPIsetRegValue PROGMEM= "SPIsetRegValue";
  const char* topicSPIwriteRegister PROGMEM= "SPIwriteRegister";
  const char* topicSPIreadRegister PROGMEM= "SPIreadRegister";
  
  
};

#endif