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

  TinyGS is an open network of Ground Stations distributed around the
  world to receive and operate LoRa satellites, weather probes and other
  flying objects, using cheap and versatile modules.

  This project is based on ESP32 boards and is compatible with sx126x and
  sx127x you can build you own board using one of these modules but most
  of us use a development board like the ones listed in the Supported
  boards section.

  Supported boards
    Heltec WiFi LoRa 32 V1 (433MHz & 863-928MHz versions)
    Heltec WiFi LoRa 32 V2 (433MHz & 863-928MHz versions)
    TTGO LoRa32 V1 (433MHz & 868-915MHz versions)
    TTGO LoRa32 V2 (433MHz & 868-915MHz versions)
    TTGO LoRa32 V2 (Manually swapped SX1267 to SX1278)
    T-BEAM + OLED (433MHz & 868-915MHz versions)
    T-BEAM V1.0 + OLED
    FOSSA 1W Ground Station (433MHz & 868-915MHz versions)
    ESP32 dev board + SX126X with crystal (Custom build, OLED optional)
    ESP32 dev board + SX126X with TCXO (Custom build, OLED optional)
    ESP32 dev board + SX127X (Custom build, OLED optional)

  Supported modules
    sx126x
    sx127x

    Web of the project: https://tinygs.com/
    Github: https://github.com/G4lile0/tinyGS
    Main community chat: https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q

    In order to onfigure your Ground Station please open a private chat to get your credentials https://t.me/tinygs_personal_bot
    Data channel (station status and received packets): https://t.me/tinyGS_Telemetry
    Test channel (simulator packets received by test groundstations): https://t.me/TinyGS_Test

    Developers:
      @gmag12       https://twitter.com/gmag12
      @dev_4m1g0    https://twitter.com/dev_4m1g0
      @g4lile0      https://twitter.com/G4lile0

====================================================
  IMPORTANT:
    - Follow this guide to get started: https://github.com/G4lile0/tinyGS/wiki/Quick-Start
    - Arduino IDE is NOT recommended, please use Platformio: https://github.com/G4lile0/tinyGS/wiki/Platformio

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

#include <cppQueue.h>
#include "src/Rotator/Rotator.h"
#include "src/Rotator/N2YO.h"

#include <ESPNtpClient.h>
#include "src/Logger/Logger.h"

#if  RADIOLIB_VERSION_MAJOR != (0x04) || RADIOLIB_VERSION_MINOR != (0x02) || RADIOLIB_VERSION_PATCH != (0x01) || RADIOLIB_VERSION_EXTRA != (0x00)
#error "You are not using the correct version of RadioLib please copy TinyGS/lib/RadioLib on Arduino/libraries"
#endif

#ifndef RADIOLIB_GODMODE
#if !PLATFORMIO
#error "Using Arduino IDE is not recommended, please follow this guide https://github.com/G4lile0/tinyGS/wiki/Arduino-IDE or edit /RadioLib/src/BuildOpt.h and uncomment #define RADIOLIB_GODMODE around line 367" 
#endif
#endif

ConfigManager& configManager = ConfigManager::getInstance();
MQTT_Client& mqtt = MQTT_Client::getInstance();
Radio& radio = Radio::getInstance();

cppQueue passes_queue(sizeof(radiopass_t), PASSESQUEUE_SIZE, FIFO, true); // instantiate the passes queue
cppQueue positions_queue(sizeof(position_t), POSITIONSQUEUE_SIZE, FIFO, false); // instantiate the positions queue

Rotator_Client& rotator = Rotator_Client::getInstance();
N2YO_Client n2yo = N2YO_Client::getInstance();
TaskHandle_t taskRotor;

const char* ntpServer = "time.cloudflare.com";
void printLocalTime();
void taskRotorHandle(void *parameter);

// Global status
Status status;

void printControls();
void switchTestmode();
void checkButton();
void setupNTP();

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

void configured()
{
  configManager.setConfiguredCallback(NULL);
  configManager.printConfig();
  radio.init();
}

void wifiConnected()
{
  configManager.setWifiConnectionCallback(NULL);
  setupNTP();
  displayShowConnected();
  arduino_ota_setup();
  configManager.delay(100); // finish animation

  if (configManager.getLowPower())
  {
    Log::debug(PSTR("Set low power CPU=80Mhz"));
    setCpuFrequencyMhz(80); //Set CPU clock to 80MHz
  }

  configManager.delay(400); // wait to show the connected screen and stabilize frequency
}

void setup()
{ 
  setCpuFrequencyMhz(240);
  Serial.begin(115200);
  delay(100);

  Log::console(PSTR("TinyGS Version %d - %s"), status.version, status.git_version);
  configManager.setWifiConnectionCallback(wifiConnected);
  configManager.setConfiguredCallback(configured);
  configManager.init();
  if (configManager.isFailSafeActive())
  {
    configManager.setConfiguredCallback(NULL);
    configManager.setWifiConnectionCallback(NULL);
    Log::console(PSTR("FATAL ERROR: The board is in a boot loop, rescue mode launched. Connect to the WiFi AP: %s, and open a web browser on ip 192.168.4.1 to fix your configuration problem or upload a new firmware."), configManager.getThingName());
    return;
  }
  // make sure to call doLoop at least once before starting to use the configManager
  configManager.doLoop();
  pinMode (configManager.getBoardConfig().PROG__BUTTON, INPUT_PULLUP);
  displayInit();
  displayShowInitialCredits();
  configManager.delay(1000);
  mqtt.begin();

  N2YO_Client::getInstance().printStats();
  xTaskCreatePinnedToCore(taskRotorHandle, "taskRotor", 10000, NULL, 1, &taskRotor, 0);                         

  if (configManager.getOledBright() == 0)
  {
    displayTurnOff();
  }
  
  printControls();
}

void loop() {  
  configManager.doLoop();
  if (configManager.isFailSafeActive())
  {
    static bool updateAttepted = false;
    if (!updateAttepted && configManager.isConnected()) {
      updateAttepted = true;
      OTA::update(); // try to update as last resource to recover from this state
    }

    if (millis() > 10000 || updateAttepted)
      configManager.forceApMode(true);
    
    return;
  }

  ArduinoOTA.handle();
  handleSerial();

  if (configManager.getState() < 2) // not ready or not configured
  {
    displayShowApMode();
    return;
  }
  
  // configured and no connection
  checkButton();
  if (radio.isReady())
  {
    status.radio_ready = true;
    radio.listen();
  }
  else {
    status.radio_ready = false;
  }

  if (configManager.getState() < 4) // connection or ap mode
  {
    displayShowStaMode(configManager.isApMode());
    return;
  }

  // connected

  mqtt.loop();
  OTA::loop();
  if (configManager.getOledBright() != 0) displayUpdate();
}

void setupNTP()
{
  NTP.setInterval (120); // Sync each 2 minutes
  NTP.setTimeZone (configManager.getTZ ()); // Get TX from config manager
  NTP.onNTPSyncEvent (ntp_cb); // Register event callback
  NTP.setMinSyncAccuracy (2000); // Sync accuracy target is 2 ms
  NTP.settimeSyncThreshold (1000); // Sync only if calculated offset absolute value is greater than 1 ms
  NTP.setMaxNumSyncRetry (2); // 2 resync trials if accuracy not reached
  NTP.begin (ntpServer); // Start NTP client
  Serial.printf ("NTP started...\n");
  
  time_t startedSync = millis ();
  while (NTP.syncStatus() != syncd && millis() - startedSync < 5000) // Wait 5 seconds to get sync
  {
    configManager.delay(100);
  }

  printLocalTime();
}

void checkButton()
{
  #define RESET_BUTTON_TIME 8000
  static unsigned long buttPressedStart = 0;
  if (!digitalRead (configManager.getBoardConfig().PROG__BUTTON))
  {
    if (!buttPressedStart)
    {
      buttPressedStart = millis();
    }
    else if (millis() - buttPressedStart > RESET_BUTTON_TIME) // long press
    {
      Log::console(PSTR("Rescue mode forced by button long press!"));
      Log::console(PSTR("Connect to the WiFi AP: %s and open a web browser on ip 192.168.4.1 to configure your station and manually reboot when you finish."), configManager.getThingName());
      configManager.forceDefaultPassword(true);
      configManager.forceApMode(true);
      buttPressedStart = 0;
    }
  }
  else {
    unsigned long elapsedTime = millis() - buttPressedStart;
    if (elapsedTime > 30 && elapsedTime < 1000) // short press
      displayNextFrame();
    buttPressedStart = 0;
  }
}

void handleSerial()
{
  if(Serial.available())
  {
    radio.disableInterrupt();

    // get the first character
    char serialCmd = Serial.read();

    // wait for a bit to receive any trailing characters
    configManager.delay(50);

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
        break;
      case 'b':
        ESP.restart();
        break;
      case 'p':
        if (!configManager.getAllowTx())
        {
          Log::console(PSTR("Radio transmission is not allowed by config! Check your config on the web panel and make sure transmission is allowed by local regulations"));
          break;
        }

        static long lastTestPacketTime = 0;
        if (millis() - lastTestPacketTime < 20*1000)
        {
          Log::console(PSTR("Please wait a few seconds to send another test packet."));
          break;
        }
        
        radio.sendTestPacket();
        lastTestPacketTime = millis();
        Log::console(PSTR("Sending test packet to nearby stations!"));
        break;
      default:
        Log::console(PSTR("Unknown command: %c"), serialCmd);
        break;
    }

    radio.enableInterrupt();
  }
}

void switchTestmode()
{  
  if (configManager.getTestMode())
  {
      configManager.setTestMode(false);
      Log::console(PSTR("Changed from test mode to normal mode"));
  }
  else
  {
      configManager.setTestMode(true);
      Log::console(PSTR("Changed from normal mode to test mode"));
  }
}

void printLocalTime()
{
    time_t currenttime = time (NULL);
    if (currenttime < 0) {
        Log::error (PSTR ("Failed to obtain time: %d"), currenttime);
        return;
    }
    struct tm* timeinfo;
    
    timeinfo = localtime (&currenttime);
  
  Serial.println(timeinfo, "%A, %B %d %Y %H:%M:%S (local)");
}

// function to print controls
void printControls()
{
  Log::console(PSTR("------------- Controls -------------"));
  Log::console(PSTR("t - change the test mode and restart"));
  Log::console(PSTR("e - erase board config and reset"));
  Log::console(PSTR("b - reboot the board"));
  Log::console(PSTR("p - send test packet to nearby stations (to check transmission)"));
  Log::console(PSTR("------------------------------------"));
}

void taskRotorHandle(void *parameter)
{
  position_t position;
  radiopass_t radiopass;

  Log::debug(PSTR("taskRotor is running on core %d"), xPortGetCoreID());

  // WARNING! not thread-safe!

  for (;;)
  {
    time_t utc;
    struct tm *currenttime;

    // il positions queue is not empty, process it...
    while (!positions_queue.isEmpty())
    {
      time(&utc);
      currenttime = gmtime(&utc);
      // Log::debug(PSTR("current UTC: timestamp=%ld %04d/%02d/%02d %02d:%02d:%02d (UTC)"), utc, 1900 + currenttime->tm_year, currenttime->tm_mon, currenttime->tm_mday, currenttime->tm_hour, currenttime->tm_min, currenttime->tm_sec);

      positions_queue.peek(&position);

      if (position.timestamp < utc)
      {
        Log::debug(PSTR("NORAD SAT#%lu: position timestamp %ld is %ld seconds in the past... and has been ignored..."), position.norad_id, position.timestamp, utc - position.timestamp);
        positions_queue.drop();
        continue;
      }

      // WARNING! rischio di dormire troppo a lungo ?
      // WARNING! la coda delle posizioni dovrebbe essere resettata se arriva una nuova richiesta!
      // WARNING! mentre aspetto potrei puntare genericamente verso lo zenith oppure sul punto di massima elevazione fornito da N2YO
      // WARNING! prevedere di poter uscire dallo sleep di 30s... servirebbe una terminate oppure rivedere il codice affinche' continui a girare ogni secondo e stampi ogni 60s...
      if (position.timestamp > utc)
      {
        int delta = position.timestamp - utc;
        Log::debug(PSTR("NORAD SAT#%lu: position timestamp %ld is %ld seconds in the future... waiting..."), position.norad_id, position.timestamp, delta);
        if (delta > 60)
          delta = 60; // wake up every minute...
        if (delta >= 1)
          delay((1000 * delta) - 100); // just wakeup a little bit earlier...
        continue;
      }

      struct tm *postime = gmtime(&position.timestamp);
      Log::debug(PSTR("tracking NORAD SAT#%lu timestamp=%ld UTC:%04d/%02d/%02d %02d:%02d:%02d latitude=%f longitude=%f altitude=%f azimuth=%f elevation=%f"), position.norad_id, position.timestamp, 1900 + postime->tm_year, postime->tm_mon, postime->tm_mday, postime->tm_hour, postime->tm_min, postime->tm_sec, position.sat_latitude, position.sat_longitude, position.sat_altitude, position.azimuth, position.elevation);

      positions_queue.drop();
    }

    // if we are here, positions_queue is empty...
    // if we have radiopasses in the radiopasses-queue, extract the first one and query N2YO for SAT positions...
    if (passes_queue.isEmpty())
    {
      // Log::debug(PSTR("radio-passes queue is empty..."));
      delay(1000);
      continue;
    }

    Log::debug(PSTR("checking first item in radio-passes queue..."));

    passes_queue.peek(&radiopass);

    time(&utc);
    currenttime = gmtime(&utc);

    // if the radiopass has elapsed... just remove it from the radio-pass queue...
    if (utc >= radiopass.endUTC)
    {
      Log::debug(PSTR("radio-pass with endUTC %ld has expired..."), radiopass.endUTC);
      passes_queue.drop();
      continue;
    }

    // WARNING! rischio di dormire troppo a lungo ?
    // WARNING! la coda dei passaggi radio dovrebbe essere resettata se arriva una nuova richiesta!
    if (utc < radiopass.startUTC)
    {
      int delta = radiopass.startUTC - utc;
      Log::debug(PSTR("radio-pass with startUTC %ld is %ld seconds in the future... waiting..."), radiopass.startUTC, delta);
      if (delta > 60)
        delta = 60; // wake up every minute...
      if (delta >= 1)
        delay((1000 * delta) - 100); // just wakeup a little bit earlier...
      continue;
    }

    // query N2YO for the GPS positions for the given NORAD id; results will be automatically pushed in the positions_queue
    N2YO_Client::getInstance().query_positions(radiopass.norad_id);

    delay(500);
  }
}