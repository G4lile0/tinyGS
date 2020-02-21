/*
  Radio.h - Class to handle radio communications
  
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

#ifndef RADIO_H
#define RADIO_H

#include <RadioLib.h>
#include "../ConfigManager/ConfigManager.h"
#include "../Status.h"
#include "../Mqtt/MQTT_Client.h"

extern Status status;

class Radio {
public:
  static Radio& getInstance()
  {
    static Radio instance; 
    return instance;
  }

  void init();
  void sendPing();
  void requestInfo();
  void requestPacketInfo();
  void requestRetransmit(char* data);
  void enableInterrupt();
  void disableInterrupt();
  uint8_t listen();
  bool isReady() { return ready; }
  void remote_freq(char* payload, size_t payload_len);
  void remote_bw(char* payload, size_t payload_len);
  void remote_sf(char* payload, size_t payload_len);
  void remote_cr(char* payload, size_t payload_len);
  void remote_crc(char* payload, size_t payload_len);
  void remote_pl(char* payload, size_t payload_len);
  void remote_begin_lora(char* payload, size_t payload_len);

private:
  Radio();
  PhysicalLayer* lora;
  void processReceivedFrame(uint8_t functionId, uint8_t *respOptData, size_t respLen);
  
  static void setFlag();
  int sendFrame(uint8_t functionId, const char* data = "");
  bool ready = false;
  SPIClass spi;
};

#endif