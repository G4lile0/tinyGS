/*
  ArduinoOTA.cpp - functions to handle Arduino OTA Update
  
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

constexpr auto TAG = "ARDUINOOTA";

#include "ArduinoOTA.h"

void arduino_ota_setup () {
	ArduinoOTA
		.onStart ([]() {
			String type;
			if (ArduinoOTA.getCommand () == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			  // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			ESP_LOGD (TAG, "Start updating %s", type.c_str());
		})
		.onEnd ([]() {
			ESP_LOGD (TAG, "End");
				})
		.onProgress ([](unsigned int progress, unsigned int total) {
			static uint8_t lastValue = 255;
			uint8_t nextValue = progress / (total / 100);
			if (lastValue != nextValue) {
				ESP_LOGD (TAG, "Progress: %u%%\r", nextValue);
				lastValue = nextValue;
			}
		})
		.onError ([](ota_error_t error) {
			ESP_LOGE (TAG, "Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) ESP_LOGE (TAG, "Auth Failed");
			else if (error == OTA_BEGIN_ERROR) ESP_LOGE (TAG, "Begin Failed");
			else if (error == OTA_CONNECT_ERROR) ESP_LOGE (TAG, "Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) ESP_LOGE (TAG, "Receive Failed");
			else if (error == OTA_END_ERROR) ESP_LOGE (TAG, "End Failed");
		});

	ArduinoOTA.setHostname ("FossaStation");

	ArduinoOTA.begin ();
}
