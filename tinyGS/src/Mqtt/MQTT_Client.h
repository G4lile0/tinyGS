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
  void sendRx(String packet);
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
  String buildTopic(const char * baseTopic, const char * cmnd);
  void subscribeToAll();
  void manageSatPosOled(char* payload, size_t payload_len);
  void remoteSatCmnd(char* payload, size_t payload_len);

  unsigned long lastPing = 0;
  unsigned long lastConnectionAtempt = 0;
  uint8_t connectionAtempts = 0;

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
  // GOD MODE  With great power comes great responsibility!
  const char* commandSPIsetRegValue PROGMEM= "SPIsetRegValue";
  const char* commandSPIwriteRegister PROGMEM= "SPIwriteRegister";
  const char* commandSPIreadRegister PROGMEM= "SPIreadRegister";
};

#endif