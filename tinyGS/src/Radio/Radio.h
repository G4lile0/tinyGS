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
  void enableInterrupt();
  void disableInterrupt();
  uint8_t listen();
  bool isReady() { return ready; }
  void remote_freq(char* payload, size_t payload_len);
  void remote_bw(char* payload, size_t payload_len);
  void remote_sf(char* payload, size_t payload_len);
  void remote_cr(char* payload, size_t payload_len);
  void remote_crc(char* payload, size_t payload_len);
  void remote_lsw(char* payload, size_t payload_len);
  void remote_fldro(char* payload, size_t payload_len);
  void remote_aldro(char* payload, size_t payload_len);
  void remote_pl(char* payload, size_t payload_len);
  void remote_begin_lora(char* payload, size_t payload_len);
  void remote_begin_fsk(char* payload, size_t payload_len);
  void remote_br(char* payload, size_t payload_len);
  void remote_fd(char* payload, size_t payload_len);
  void remote_fbw(char* payload, size_t payload_len);
  void remote_fsw(char* payload, size_t payload_len);
  void remote_fook(char* payload, size_t payload_len);
  void remote_SPIsetRegValue(char* payload, size_t payload_len);
  void remote_SPIwriteRegister(char* payload, size_t payload_len);
  void remote_SPIreadRegister(char* payload, size_t payload_len);
  int sendTx(const char* data, size_t length);
  
   
private:
  Radio();
  PhysicalLayer* lora;
  void readState(int state);
  static void setFlag();
  bool ready = false;
  SPIClass spi;
};

// Code from fossa
#define PRINT_BUFF(BUFF, LEN) { \
  for(size_t i = 0; i < LEN; i++) { \
    Serial.print(F("0x")); \
    Serial.print(BUFF[i], HEX); \
    Serial.print('\t'); \
    Serial.write(BUFF[i]); \
    Serial.println(); \
  } }

#endif