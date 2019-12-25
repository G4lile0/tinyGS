/***********************************************************************
  FossaGroundStation.ini - GroundStation firmware
  
  Copyright (C) 2020 @G4lile0, @gmag12 y @dev_4m1g0

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

**************************************************************************/

#include <Arduino.h>
#include "ConfigManager/ConfigManager.h"

ConfigManager configManager;

void wifiConnected() {
  Serial.print("MQTT Port: ");
  Serial.println(configManager.getMqttPort());
  Serial.print("MQTT Server: ");
  Serial.println(configManager.getMqttServer());
  Serial.print("MQTT Pass: ");
  Serial.println(configManager.getMqttPass());
  Serial.print("Latitude: ");
  Serial.println(configManager.getLatitude());
  Serial.print("Longitude: ");
  Serial.println(configManager.getLongitude());
  Serial.print("tz: ");
  Serial.println(configManager.getTZ());
  Serial.print("board: ");
  Serial.println(configManager.getBoard());
  
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");
  configManager.setWifiConnectionCallback(wifiConnected);
  configManager.init();
}

void loop() {
  configManager.doLoop();
}