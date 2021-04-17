/*
  Radio.h - Class to handle radio communications
  
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

#ifndef RADIO_H
#define RADIO_H
#define RADIOLIB_EXCLUDE_HTTP

#ifndef RADIOLIB_GODMODE
#define RADIOLIB_GODMODE
#endif
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
  int16_t begin();
  void enableInterrupt();
  void disableInterrupt();
  void startRx();
  uint8_t listen();
  bool isReady() { return status.radio_ready; }
  int16_t remote_freq(char* payload, size_t payload_len);
  int16_t remote_bw(char* payload, size_t payload_len);
  int16_t remote_sf(char* payload, size_t payload_len);
  int16_t remote_cr(char* payload, size_t payload_len);
  int16_t remote_crc(char* payload, size_t payload_len);
  int16_t remote_lsw(char* payload, size_t payload_len);
  int16_t remote_fldro(char* payload, size_t payload_len);
  int16_t remote_aldro(char* payload, size_t payload_len);
  int16_t remote_pl(char* payload, size_t payload_len);
  int16_t remote_begin_lora(char* payload, size_t payload_len);
  int16_t remote_begin_fsk(char* payload, size_t payload_len);
  int16_t remote_br(char* payload, size_t payload_len);
  int16_t remote_fd(char* payload, size_t payload_len);
  int16_t remote_fbw(char* payload, size_t payload_len);
  int16_t remote_fsw(char* payload, size_t payload_len);
  int16_t remote_fook(char* payload, size_t payload_len);
  int16_t remote_SPIsetRegValue(char* payload, size_t payload_len);
  void remote_SPIwriteRegister(char* payload, size_t payload_len);
  int16_t remote_SPIreadRegister(char* payload, size_t payload_len);
  int16_t sendTx(uint8_t* data, size_t length);
  int16_t sendTestPacket();
   
private:
  Radio();
  PhysicalLayer* lora;
  void readState(int state);
  static void setFlag();
  SPIClass spi;
  const char* TEST_STRING = "TinyGS-test "; // make sure this always start with "TinyGS-test"!!!

  double _atof(const char* buff, size_t length);
  int _atoi(const char* buff, size_t length);
};

#endif