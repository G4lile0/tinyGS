/*
  Rotator.h - Class to handle rotator communications
  
  Copyright (C) 2021 @iw2lsi

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

#ifndef ROTATOR_H
#define ROTATOR_H

#include "../ConfigManager/ConfigManager.h"
#include "../Status.h"

#if MQTT_MAX_PACKET_SIZE != 1000  && !PLATFORMIO
#error "Using Arduino IDE is not recommended, please follow this guide https://github.com/G4lile0/tinyGS/wiki/Arduino-IDE or edit /PubSubClient/src/PubSubClient.h  and set #define MQTT_MAX_PACKET_SIZE 1000"
#endif

extern Status status;

class Rotator_Client
{
public:

  static Rotator_Client& getInstance()
  {
    static Rotator_Client instance; 
    return instance;
  }

//void begin();
//void loop();

protected:

//void reconnect();

private:

  Rotator_Client();

};

#endif