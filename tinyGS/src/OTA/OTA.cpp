/*
  OTA.cpp - On The Air Update Class
  
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

#include "OTA.h"
#include "../ConfigManager/ConfigManager.h"
#include "../Status.h"

extern Status status;

void OTA::update()
{
#ifdef SECURE_OTA
    WiFiClientSecure client;
    client.setCACert(DSTroot_CA_update);
#else
    WiFiClient client;
#endif

  Serial.print("Checking for OTA Updates...  ");
  uint64_t chipId = ESP.getEfuseMac();
  char clientId[13];
  sprintf(clientId, "%04X%08X",(uint16_t)(chipId>>32), (uint32_t)chipId);

  t_httpUpdate_return ret = httpUpdate.update(client, String(OTA_URL) + clientId, status.git_version);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("Update failed Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES: // server 304
      Serial.println("No updates required");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("Update ok but ESP has not restarted!!! (This should never be printed)");
      break;
  }
}

unsigned static long lastUpdateTime = 0;
void OTA::loop()
{
  if (millis() < MIN_TIME_BEFORE_UPDATE || !ConfigManager::getInstance().getAutoUpdate())
    return;

  if (millis() - lastUpdateTime > TIME_BETTWEN_UPDATE_CHECK)
  {
    lastUpdateTime = millis();
    update();
  }
}
