/*
  OTA.h - On The Air Update Class
  
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

#ifndef OTA_H
#define OTA_H

#define SECURE_OTA // Comment this line if you are not using SSL for OTA (Not recommended)

#include <HTTPClient.h>
#include <HTTPUpdate.h>

constexpr auto MIN_TIME_BEFORE_UPDATE = 20000;
constexpr auto TIME_BETTWEN_UPDATE_CHECK = 3600000;
constexpr auto OTA_URL = "https://ota.tinygs.com/updates/tinygs.bin";

#ifdef SECURE_OTA
#include "../certs.h"
#endif

class OTA
{
public:
  static void loop();
  static void update();
};

#endif