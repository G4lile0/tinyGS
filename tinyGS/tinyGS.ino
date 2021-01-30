/***********************************************************************
  tinyGS.ini - GroundStation firmware
  
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

  ***********************************************************************

  The aim of this project is to create an open network of ground stations
  for the LoRa Satellites distributed all over the world and connected
  through Internet.
  This project is based on ESP32 boards and is compatible with sx126x and
  sx127x you can build you own board using one of these modules but most
  of us use a development board like the ones listed in the Supported
  boards section.
  The developers of this project have no relation with the Fossa team in
  charge of the mission, we are passionate about space and created this
  project to be able to track and use the satellites as well as supporting
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
#include "src/Display/Display.h"
#include "src/Mqtt/MQTT_Client.h"
#include "src/Status.h"
#include "src/Radio/Radio.h"
#include "src/ArduinoOTA/ArduinoOTA.h"
#include "src/OTA/OTA.h"
#include <ESPNtpClient.h>
#include <FailSafe.h>

#if MQTT_MAX_PACKET_SIZE != 1000
"Remeber to change libraries/PubSubClient/src/PubSubClient.h"
        "#define MQTT_MAX_PACKET_SIZE 1000"
#endif

#if  RADIOLIB_VERSION_MAJOR != (0x04) || RADIOLIB_VERSION_MINOR != (0x01) || RADIOLIB_VERSION_PATCH != (0x01) || RADIOLIB_VERSION_EXTRA != (0x00)
#error "You are not using the correct version of RadioLib please copy TinyGS/lib/RadioLib on Arduino/libraries"
#endif

#ifndef RADIOLIB_GODMODE
#error "Seems you are using Arduino IDE, edit /RadioLib/src/BuildOpt.h and uncomment #define RADIOLIB_GODMODE around line 367" 
#endif


const int MAX_CONSECUTIVE_BOOT = 3; // Number of rapid boot cycles before enabling fail safe mode
const time_t BOOT_FLAG_TIMEOUT = 10000; // Time in ms to reset fail safe mode activation flag

ConfigManager& configManager = ConfigManager::getInstance();
MQTT_Client& mqtt = MQTT_Client::getInstance();
Radio& radio = Radio::getInstance();

TaskHandle_t dispUpdate_handle;

const char* ntpServer = "time.cloudflare.com";
void printLocalTime();

// Global status
Status status;

void printControls();
void switchTestmode();

void ntp_cb (NTPEvent_t e)
{
  switch (e.event) {
    case timeSyncd:
    case partlySync:
      //Serial.printf ("[NTP Event] %s\n", NTP.ntpEvent2str (e));
      status.time_offset = e.info.offset;
      break;
    default:
      break;
  }
}

void displayUpdate_task (void* arg)
{
  for (;;){
      displayUpdate ();
  }
}

void wifiConnected()
{
  configManager.printConfig();
  arduino_ota_setup();
  displayShowConnected();

  radio.init();
  if (!radio.isReady())
  {
    Serial.println("LoRa initialization failed. Please connect to " + WiFi.localIP().toString() + " and make sure the board selected matches your hardware.");
    displayShowLoRaError();
  }

  NTP.setInterval (120); // Sync each 2 minutes
  NTP.setTimeZone (configManager.getTZ ()); // Get TX from config manager
  NTP.onNTPSyncEvent (ntp_cb); // Register event callback
  NTP.setMinSyncAccuracy (2000); // Sync accuracy target is 2 ms
  NTP.settimeSyncThreshold (1000); // Sync only if calculated offset absolute value is greater than 1 ms
  NTP.setMaxNumSyncRetry (2); // 2 resync trials if accuracy not reached
  NTP.begin (ntpServer); // Start NTP client
  
  time_t startedSync = millis ();
  while (NTP.syncStatus() != syncd && millis() - startedSync < 5000) // Wait 5 seconds to get sync
  {
      delay (100);
  }

  printLocalTime();
  configManager.delay(1000); // wait to show the connected screen

  mqtt.begin();

  // TODO: Make this beautiful
  displayShowWaitingMqttConnection();
  printControls();
}

void setup()
{
  Serial.begin(115200);
  delay(299);

  FailSafe.checkBoot (MAX_CONSECUTIVE_BOOT); // Parameters are optional
  if (FailSafe.isActive ()) // Skip all user setup if fail safe mode is activated
      return;

  Serial.printf("TinyGS Version %d - %s\n", status.version, status.git_version);
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
  while (millis () - start_waiting_for_button < WAIT_FOR_BUTTON)
  {
	  if (!digitalRead (configManager.getBoardConfig().PROG__BUTTON))
    {
		  button_pushed = true;
		  button_pushed_at = millis ();
		  ESP_LOGI (LOG_TAG, "Reset button pushed");
		  while (millis () - button_pushed_at < RESET_BUTTON_TIME)
      {
			  if (digitalRead (configManager.getBoardConfig().PROG__BUTTON))
        {
				  ESP_LOGI (LOG_TAG, "Reset button released");
				  button_pushed = false;
				  break;
			  }
		  }
		  if (button_pushed)
      {
			  ESP_LOGI (LOG_TAG, "Reset config triggered");
			  WiFi.begin ("0", "0");
			  WiFi.disconnect ();
		  }
	  }
  }

  if (button_pushed)
  {
    configManager.resetAPConfig();
    ESP.restart();
  }
  
  if (configManager.isApMode())
    displayShowApMode();
  else 
    displayShowStaMode();
  
  delay(500);  
}

void loop() {
  static bool startDisplayTask = true;
    
  FailSafe.loop (BOOT_FLAG_TIMEOUT); // Use always this line
  if (FailSafe.isActive ()) // Skip all user loop code if Fail Safe mode is active
    return;
    
  configManager.doLoop();

  static bool wasConnected = false;
  if (!configManager.isConnected())
  {
    if (wasConnected)
    {
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
  OTA::loop();
  
  if(Serial.available())
  {
    radio.disableInterrupt();

    // get the first character
    char serialCmd = Serial.read();

    // wait for a bit to receive any trailing characters
    delay(50);

    // dump the serial buffer
    while(Serial.available())
    {
      Serial.read();
    }

    // process serial command
    switch(serialCmd) {
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
      case 'p':
        static long lastTestPacketTime = 0;
        if (millis() - lastTestPacketTime < 20*1000)
        {
          Serial.println(F("Please wait a few seconds to send another test packet."));
          break;
        }
        radio.sendTestPacket();
        break;
      default:
        Serial.print(F("Unknown command: "));
        Serial.println(serialCmd);
        break;
    }

    radio.enableInterrupt();
  }

  if (!radio.isReady())
  {
    displayShowLoRaError();
    return;
  }

  if (startDisplayTask)
  {
    startDisplayTask = false;
    xTaskCreateUniversal (
            displayUpdate_task,           // Display loop function
            "Display Update",             // Task name
            4096,                         // Stack size
            NULL,                         // Function argument, not needed
            1,                            // Priority, running higher than 1 causes errors on MQTT comms
            &dispUpdate_handle,           // Task handle
            CONFIG_ARDUINO_RUNNING_CORE); // Running core, should be 1
  }

  radio.listen();
}

void switchTestmode()
{  
  if (configManager.getTestMode())
  {
      configManager.setTestMode(false);
      Serial.println(F("Changed from test mode to normal mode"));
  }
  else
  {
      configManager.setTestMode(false);
      Serial.println(F("Changed from normal mode to test mode"));
  }
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// function to print controls
void printControls()
{
  Serial.println(F("------------- Controls -------------"));
  Serial.println(F("t - change the test mode and restart"));
  Serial.println(F("e - erase board config and reset"));
  Serial.println(F("b - reboot the board"));
  Serial.println(F("------------------------------------"));
}