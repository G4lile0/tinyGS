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
  void sendRx(String packet, bool noisy);
  void manageMQTTData(char *topic, uint8_t *payload, unsigned int length);
  void sendStatus();
  void scheduleRestart() { scheduledRestart = true; };

protected:
#ifdef SECURE_MQTT
  WiFiClientSecure espClient;
#else
  WiFiClient espClient;
#endif
  void reconnect();

private:
  MQTT_Client();
  String buildTopic(const char * baseTopic, const char * cmnd);
  void subscribeToAll();
  void manageSatPosOled(char* payload, size_t payload_len);
  void remoteSatCmnd(char* payload, size_t payload_len);

  unsigned long lastPing = 0;
  unsigned long lastConnectionAtempt = 0;
  uint8_t connectionAtempts = 0;
  bool scheduledRestart = false;

  const unsigned long pingInterval = 1 * 60 * 1000;
  const unsigned long reconnectionInterval = 5 * 1000;
  uint16_t connectionTimeout = 5 * 60 * 1000 / reconnectionInterval;

  const char* globalTopic PROGMEM = "tinygs/global/%cmnd%";
  const char* cmndTopic PROGMEM = "tinygs/%user%/%station%/cmnd/%cmnd%";
  const char* teleTopic PROGMEM = "tinygs/%user%/%station%/tele/%cmnd%";
  const char* statTopic PROGMEM = "tinygs/%user%/%station%/stat/%cmnd%";

  // tele
  const char* topicWelcome PROGMEM = "welcome";
  const char* topicPing PROGMEM= "ping";
  const char* topicStatus PROGMEM = "status";
  const char* topicRx PROGMEM= "rx";

  // command
  const char* commandBatchConf PROGMEM= "batch_conf";
  const char* commandUpdate PROGMEM= "update";
  const char* commandSatPos PROGMEM= "sat_pos_oled";
  const char* commandReset PROGMEM= "reset";
  const char* commandFreq PROGMEM= "freq";
  const char* commandBw PROGMEM= "bw";
  const char* commandSf PROGMEM= "sf";
  const char* commandCr PROGMEM= "cr";
  const char* commandCrc PROGMEM= "crc";
  const char* commandLsw PROGMEM= "lsw";
  const char* commandFldro PROGMEM= "fldro";
  const char* commandAldro PROGMEM= "aldro";
  const char* commandPl PROGMEM= "pl";
  const char* commandBegin PROGMEM= "begin";
  const char* commandBeginLora PROGMEM= "begin_lora";
  const char* commandBeginFSK PROGMEM= "begin_fsk";
  const char* commandBr PROGMEM= "br";
  const char* commandFd PROGMEM= "Fd";
  const char* commandFbw PROGMEM= "fbw";
  const char* commandFsw PROGMEM= "fsw";
  const char* commandFook PROGMEM= "fok";
  const char* commandFrame PROGMEM= "frame";
  const char* commandSat PROGMEM= "sat";
  const char* commandStatus PROGMEM= "status";
  const char* commandTest PROGMEM= "test";
  const char* commandRemoteTune PROGMEM= "remoteTune";
  const char* commandRemotetelemetry3rd PROGMEM= "telemetry3rd";
  const char* commandLog PROGMEM= "log";
  const char* commandTx PROGMEM= "tx";
  // GOD MODE  With great power comes great responsibility!
  const char* commandSPIsetRegValue PROGMEM= "SPIsetRegValue";
  const char* commandSPIwriteRegister PROGMEM= "SPIwriteRegister";
  const char* commandSPIreadRegister PROGMEM= "SPIreadRegister";
};

#endif