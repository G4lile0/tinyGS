/*
  MQTTClient.h - MQTT connection class
  
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

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#define SECURE_MQTT // Comment this line if you are not using MQTT over SSL
#define LOG_TAG "MQTT"

#include "../ConfigManager/ConfigManager.h"
#include "../Status.h"
#include <PubSubClient.h>
#ifdef SECURE_MQTT
#include <WiFiClientSecure.h>

static const char DSTroot_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ
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

  const char* topicStart PROGMEM = "fossa";
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
  const char* topicGlobalRemote PROGMEM= "fossa/global/remote/";
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
  const char* topicRemoteSat PROGMEM= "sat";
  const char* topicRemoteStatus PROGMEM= "status";
  const char* topicRemoteTest PROGMEM= "test";
  
  
  


};

#endif