/***********************************************************************
  FossaGroundStation.ini - GroundStation firmware
  
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

  ***********************************************************************

  The aim of this project is to create an open network of ground stations
  for the Fossa Satellites distributed all over the world and connected
  through Internet.
  This project is based on ESP32 boards and is compatible with sx126x and
  sx127x you can build you own board using one of these modules but most
  of us use a development board like the ones listed in the Supported
  boards section.
  The developers of this project have no relation with the Fossa team in
  charge of the mission, we are passionate about space and created this
  project to be able to trackand use the satellites as well as supporting
  the mission.

  Supported boards
    Heltec WiFi LoRa 32 V1 (433MHz SX1278)
    Heltec WiFi LoRa 32 V2 (433MHz SX1278)
    TTGO LoRa32 V1 (433MHz SX1278)
    TTGO LoRa32 V2 (433MHz SX1278)

  Supported modules
    sx126x
    sx127x

    World Map with active Ground Stations and satellite stimated possition 
    Main community chat: https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q

    In order to onfigure your Ground Station please open a private chat to get your credentials https://t.me/fossa_updates_bot
    Data channel (station status and received packets): https://t.me/FOSSASAT_DATA
    Test channel (simulator packets received by test groundstations): https://t.me/FOSSASAT_TEST

    Developers:
      @gmag12       https://twitter.com/gmag12
      @dev_4m1g0    https://twitter.com/dev_4m1g0
      @g4lile0      https://twitter.com/G4lile0

====================================================
  IMPORTANT:
    - Change libraries/PubSubClient/src/PubSubClient.h
        #define MQTT_MAX_PACKET_SIZE 1000

**************************************************************************/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "src/ConfigManager/ConfigManager.h"
#include "src/Comms/Comms.h"
#include "src/Display/Display.h"
#include "src/Mqtt/MQTT_Client.h"
#include "src/Status.h"
#include "src/Radio/Radio.h"
#include "src/ArduinoOTA/ArduinoOTA.h"

#if MQTT_MAX_PACKET_SIZE != 1000
"Remeber to change libraries/PubSubClient/src/PubSubClient.h"
        "#define MQTT_MAX_PACKET_SIZE 1000"
#endif

#if  RADIOLIB_VERSION_EXTRA != (0x16)
"We are using a patched version of RadioLib please copy ESP32-OLED-Fossa-GroundStation/lib/RadioLib on Arduino/libraries"
#endif


ConfigManager configManager;
MQTT_Client mqtt(configManager);
Radio radio(configManager, mqtt);

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0; // 3600;         // 3600 for Spain
const int   daylightOffset_sec = 0; // 3600;
void printLocalTime();

// Global status
Status status;

void printControls();
void switchTestmode();

void wifiConnected() {
  configManager.printConfig();
  arduino_ota_setup();
  displayShowConnected();

  radio.init();
  if (!radio.isReady()){
    Serial.println("LoRa initialization failed. Please connect to " + WiFi.localIP().toString() + " and make sure the board selected matches your hardware.");
    displayShowLoRaError();
  }

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (strcmp (configManager.getTZ(), "")) {
	  setenv("TZ", configManager.getTZ(), 1);
	  ESP_LOGD (LOG_TAG, "Set timezone value as %s", configManager.getTZ());
	  tzset();
  }

  printLocalTime();
  configManager.delay(1000); // wait to show the connected screen

  mqtt.begin();

  // TODO: Make this beautiful
  displayShowWaitingMqttConnection();
  printControls();
}

void setup() {
  Serial.begin(115200);
  delay(299);
  Serial.printf("Fossa Ground station Version %d\n", status.version);
  configManager.setWifiConnectionCallback(wifiConnected);
  configManager.init();
  // make sure to call doLoop at least once before starting to use the configManager
  configManager.doLoop();
  pinMode (configManager.getBoardConfig().PROG__BUTTON, INPUT_PULLUP);
  displayInit();
  displayShowInitialCredits();

#define WAIT_FOR_BUTTON 3000
#define RESET_BUTTON_TIME 5000
  unsigned long start_waiting_for_button = millis ();
  unsigned long button_pushed_at;
  ESP_LOGI (LOG_TAG, "Waiting for reset config button");
  bool button_pushed = false;
  while (millis () - start_waiting_for_button < WAIT_FOR_BUTTON) {

	  if (!digitalRead (configManager.getBoardConfig().PROG__BUTTON)) {
		  button_pushed = true;
		  button_pushed_at = millis ();
		  ESP_LOGI (LOG_TAG, "Reset button pushed");
		  while (millis () - button_pushed_at < RESET_BUTTON_TIME) {
			  if (digitalRead (configManager.getBoardConfig().PROG__BUTTON)) {
				  ESP_LOGI (LOG_TAG, "Reset button released");
				  button_pushed = false;
				  break;
			  }
		  }
		  if (button_pushed) {
			  ESP_LOGI (LOG_TAG, "Reset config triggered");
			  WiFi.begin ("0", "0");
			  WiFi.disconnect ();
		  }
	  }
  }

  if (button_pushed) {
    configManager.resetAPConfig();
    ESP.restart();
  }
  
  if (configManager.isApMode()) {
    displayShowApMode();
  } 
  else {
    displayShowStaMode();
  }
  delay(500);  
}

void loop() {
  configManager.doLoop();

  static bool wasConnected = false;
  if (!configManager.isConnected()){
    if (wasConnected){
      if (configManager.isApMode())
        displayShowApMode();
      else 
        displayShowStaMode();
    }
    return;
  }
  wasConnected = true;
  mqtt.loop();
  ArduinoOTA.handle();
  
  if(Serial.available()) {
    radio.disableInterrupt();

    // get the first character
    char serialCmd = Serial.read();

    // wait for a bit to receive any trailing characters
    delay(50);

    // dump the serial buffer
    while(Serial.available()) {
      Serial.read();
    }

    // process serial command
    switch(serialCmd) {
      case 'p':
        if (!radio.isReady()) {
          Serial.println(F("Radio is not ready, please configure it properly before using this command."));
          break;
        }
        radio.sendPing();
        break;
      case 'i':
        if (!radio.isReady()) {
          Serial.println(F("Radio is not ready, please configure it properly before using this command."));
          break;
        }
        radio.requestInfo();
        break;
      case 'l':
        if (!radio.isReady()) {
          Serial.println(F("Radio is not ready, please configure it properly before using this command."));
          break;
        }
        radio.requestPacketInfo();
        break;
      case 'r':
        if (!radio.isReady()) {
          Serial.println(F("Radio is not ready, please configure it properly before using this command."));
          break;
        }
        Serial.println(F("Enter message to be sent:"));
        Serial.println(F("(max 32 characters, end with LF or CR+LF)"));
        {
          // get data to be retransmited
          char optData[32];
          uint8_t bufferPos = 0;
          while(bufferPos < 32) {
            while(!Serial.available());
            char c = Serial.read();
            Serial.print(c);
            if((c != '\r') && (c != '\n')) {
              optData[bufferPos] = c;
              bufferPos++;
            } else {
              break;
            }
          }
          optData[bufferPos] = '\0';

          // wait for a bit to receive any trailing characters
          delay(100);

          // dump the serial buffer
          while(Serial.available()) {
            Serial.read();
          }

          Serial.println();
          radio.requestRetransmit(optData);
        }
        break;
      case 'e':
        configManager.resetAllConfig();
        ESP.restart();
        break;
      case 't':
        switchTestmode();
        ESP.restart();
        break;
      case 'b':
        ESP.restart();
        break;
       
      default:
        Serial.print(F("Unknown command: "));
        Serial.println(serialCmd);
        break;
    }

    radio.enableInterrupt();
  }

  if (!radio.isReady()) {
    displayShowLoRaError();
    return;
  }

  displayUpdate();

  radio.listen();
}

void switchTestmode() {
  char temp_station[32];
  if ((configManager.getThingName()[0]=='t') &&  (configManager.getThingName()[1]=='e') && (configManager.getThingName()[2]=='s') && (configManager.getThingName()[4]=='_')) {
    Serial.println(F("Changed from test mode to normal mode"));
    for (byte a=5; a<=strlen(configManager.getThingName()); a++ ) {
      configManager.getThingName()[a-5]=configManager.getThingName()[a];
    }
  }
  else
  {
    strcpy(temp_station,"test_");
    strcat(temp_station,configManager.getThingName());
    strcpy(configManager.getThingName(),temp_station);
    Serial.println(F("Changed from normal mode to test mode"));
  }

  configManager.configSave();
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// function to print controls
void printControls() {
  Serial.println(F("------------- Controls -------------"));
  Serial.println(F("p - send ping frame"));
  Serial.println(F("i - request satellite info"));
  Serial.println(F("l - request last packet info"));
  Serial.println(F("r - send message to be retransmitted"));
  Serial.println(F("t - change the test mode and restart"));
  Serial.println(F("e - erase board config and reset"));
  Serial.println(F("b - reboot the board"));
  Serial.println(F("------------------------------------"));
}