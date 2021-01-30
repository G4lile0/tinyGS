/*
  Radio.cpp - Class to handle radio communications
  
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

#include "Radio.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"
#include <base64.h>

bool received = false;
bool eInterrupt = true;

// modem configuration
#define LORA_CARRIER_FREQUENCY        436.703f  // MHz
#define LORA_BANDWIDTH                250.0f  // kHz dual sideband
#define LORA_SPREADING_FACTOR         10
#define LORA_SPREADING_FACTOR_ALT     10
#define LORA_CODING_RATE              5       // 4/8, Extended Hamming
#define LORA_OUTPUT_POWER             5       // dBm
#define LORA_CURRENT_LIMIT_7X         120     // mA
#define LORA_CURRENT_LIMIT_6X         120.0f     // mA
#define SYNC_WORD                     0x12    // sync word 
#define LORA_PREAMBLE_LENGTH          8U

Radio::Radio()
: spi(VSPI)
{
  
}

void Radio::init()
{
  Serial.print(F("[SX12xx] Initializing ... "));
  board_type board = ConfigManager::getInstance().getBoardConfig();
  
  spi.begin(board.L_SCK, board.L_MISO, board.L_MOSI, board.L_NSS);
  uint16_t state = 0;

  if (board.L_SX127X)
  {
    lora = new SX1278(new Module(board.L_NSS, board.L_DI00, board.L_DI01, spi, SPISettings(2000000, MSBFIRST, SPI_MODE0)));
    state = ((SX1278*)lora)->begin(LORA_CARRIER_FREQUENCY, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODING_RATE, SYNC_WORD, LORA_OUTPUT_POWER);
    ((SX1278*)lora)->forceLDRO(true);
    ((SX1278*)lora)->setCRC(true);
  }
  else
  {
    lora = new SX1268(new Module(board.L_NSS, board.L_DI01, board.L_RST, board.L_BUSSY, spi, SPISettings(2000000, MSBFIRST, SPI_MODE0)));
    state = ((SX1268*)lora)->begin(LORA_CARRIER_FREQUENCY, LORA_BANDWIDTH, LORA_SPREADING_FACTOR, LORA_CODING_RATE, SYNC_WORD, LORA_OUTPUT_POWER, LORA_PREAMBLE_LENGTH, board.L_TCXO_V);
    ((SX1268*)lora)->forceLDRO(true);
    ((SX1268*)lora)->setCRC(true);
  }
  
  if (state == ERR_NONE)
  {
    Serial.println(F("success!"));
  } 
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }

  // set the function that will be called
  // when new packet is received
  // attach the ISR to radio interrupt
  if (board.L_SX127X)
  {
    ((SX1278*)lora)->setDio0Action(setFlag);
  }
  else
  {
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  // start listening for LoRa packets
  Serial.print(F("[SX12x8] Starting to listen ... "));
  if (board.L_SX127X)
    state = ((SX1278*)lora)->startReceive();
  else
    state = ((SX1268*)lora)->startReceive();

  if (state == ERR_NONE)
  {
    Serial.println(F("success!"));
  } 
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    Serial.println(String("Go to the config panel (") + WiFi.localIP().toString() + ") and check if the board selected matches your hardware.");
    return;
  }

  ready = true;
}

void Radio::setFlag()
{
  if(!eInterrupt)
    return;

  received = true;
}

void Radio::enableInterrupt()
{
  eInterrupt = true;
}

void Radio::disableInterrupt()
{
  eInterrupt = false;
}

uint16_t Radio::sendTx(const char* data, size_t length)
{
  if (!ConfigManager::getInstance().getAllowTx())
  {
      Serial.println(F("TX disabled by config"));
      return -1;
  }

  // send data
  uint16_t state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    SX1278* l = (SX1278*)lora;
    state = l->transmit(data, length);
    l->setDio0Action(setFlag);
    l->startReceive();
  }
  else
  {
    SX1268* l = (SX1268*)lora;
    state = l->transmit(data, length);
    l->setDio1Action(setFlag);
    l->startReceive();
  }

  return state;
}

uint16_t Radio::sendTestPacket()
{
  return sendTx(TEST_STRING, strlen(TEST_STRING) + 1);
}

uint8_t Radio::listen()
{
  // check if the flag is set (received interruption)
  if(!received) 
    return 1;

  // disable the interrupt service routine while
  // processing the data
  disableInterrupt();

  // reset flag
  received = false;

  size_t respLen = 0;
  uint8_t* respFrame = 0;
  int state = 0;
  status.lastPacketInfo.crc_error = 0;
  // read received data
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    SX1278* l = (SX1278*)lora;
    respLen = l->getPacketLength();
    respFrame = new uint8_t[respLen];
    state = l->readData(respFrame, respLen);
    status.lastPacketInfo.rssi = l->getRSSI();
    status.lastPacketInfo.snr = l->getSNR();
    status.lastPacketInfo.frequencyerror = l->getFrequencyError();
  }
  else
  {
    SX1268* l = (SX1268*)lora;
    respLen = l->getPacketLength();
    respFrame = new uint8_t[respLen];
    state = l->readData(respFrame, respLen);
    status.lastPacketInfo.rssi = l->getRSSI();
    status.lastPacketInfo.snr = l->getSNR();
  }
 

  if ((respLen > 0) && !(state == ERR_CRC_MISMATCH))
  {
    // read optional data
    Serial.print(F("Packet ("));
    Serial.print(respLen);
    Serial.println(F(" bytes):"));
    PRINT_BUFF(respFrame, respLen)
  }

  if (state == ERR_NONE)
  {
    status.lastPacketInfo.crc_error = false;
    String encoded = base64::encode(respFrame, respLen); 
    MQTT_Client::getInstance().sendRx(encoded);
  }
  else if (state == ERR_CRC_MISMATCH) 
  {
    // packet was received, but is malformed
    status.lastPacketInfo.crc_error = true;
    String error_encoded = base64::encode("Error_CRC");
    MQTT_Client::getInstance().sendRx(error_encoded);
  } 

  delete[] respFrame;

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    status.lastPacketInfo.time = "";
  }
  else
  {
    // store time of the last packet received:
    String thisTime="";
    if (timeinfo.tm_hour < 10){ thisTime=thisTime + " ";} // add leading space if required
    thisTime=String(timeinfo.tm_hour) + ":";
    if (timeinfo.tm_min < 10){ thisTime=thisTime + "0";} // add leading zero if required
    thisTime=thisTime + String(timeinfo.tm_min) + ":";
    if (timeinfo.tm_sec < 10){ thisTime=thisTime + "0";} // add leading zero if required
    thisTime=thisTime + String(timeinfo.tm_sec);
    // const char* newTime = (const char*) thisTime.c_str();
    
    status.lastPacketInfo.time = thisTime;
  }

  // print RSSI (Received Signal Strength Indicator)
  Serial.print(F("[SX12x8] RSSI:\t\t"));
  Serial.print(status.lastPacketInfo.rssi);
  Serial.println(F(" dBm"));

  // print SNR (Signal-to-Noise Ratio)
  Serial.print(F("[SX12x8] SNR:\t\t"));
  Serial.print(status.lastPacketInfo.snr);
  Serial.println(F(" dB"));

  // print frequency error
  Serial.print(F("[SX12x8] Frequency error:\t"));
  Serial.print(status.lastPacketInfo.frequencyerror);
  Serial.println(F(" Hz"));


  // put module back to listen mode
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    ((SX1278*)lora)->startReceive();
  else
    ((SX1268*)lora)->startReceive();

  // we're ready to receive more packets,
  // enable interrupt service routine
  enableInterrupt();

  if (state == ERR_NONE)
  {
    return 0;
  }
  else if (state == ERR_CRC_MISMATCH)
  {
    // packet was received, but is malformed
    Serial.println(F("[SX12x8] CRC error!"));
    return 2;
  }
  else
  {
    // some other error occurred
    Serial.print(F("[SX12x8] Failed, code "));
    Serial.println(state);
    return 3;
  }
}

void Radio::readState(int state)
{
  if (state == ERR_NONE)
  {
    Serial.println(F("success!"));
  } 
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}

// remote
int16_t Radio::remote_freq(char* payload, size_t payload_len)
{
  float frequency = _atof(payload, payload_len);

  Serial.println("");
  Serial.print(F("Set Frequency: ")); Serial.print(frequency, 3);Serial.println(F(" MHz"));
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X) 
  {
    ((SX1278*)lora)->sleep();   // sleep mandatory if FastHop isn't ON.
    state = ((SX1278*)lora)->setFrequency(frequency);
    ((SX1278*)lora)->startReceive();
  }      
  else 
  {
    ((SX1268*)lora)->sleep();
    state = ((SX1268*)lora)->setFrequency(frequency);
    ((SX1268*)lora)->startReceive();
  }
 
  readState(state);
  if (state == ERR_NONE)
    status.modeminfo.frequency  = frequency;
  
  return state;
}

int16_t Radio::remote_bw(char* payload, size_t payload_len)
{
  float bw = _atof(payload, payload_len);

  Serial.println("");
  Serial.print(F("Set bandwidth: ")); Serial.print(bw, 3);Serial.println(F(" kHz"));
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->setBandwidth(bw);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag);
  }
  else
  {
    state = ((SX1268*)lora)->setBandwidth(bw);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  readState(state);
  if (state == ERR_NONE)
    status.modeminfo.bw = bw;
  
  return state;
}

int16_t Radio::remote_sf(char* payload, size_t payload_len)
{
  uint8_t sf = _atof(payload, payload_len);

  Serial.println("");
  Serial.print(F("Set spreading factor: ")); Serial.println(sf);
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->setSpreadingFactor(sf);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag);
  } 
  else
  {
    state = ((SX1268*)lora)->setSpreadingFactor(sf);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  readState(state);

  if (state == ERR_NONE)
    status.modeminfo.sf = sf;

  return state;
}

int16_t Radio::remote_cr(char* payload, size_t payload_len)
{
  uint8_t cr = _atoi(payload, payload_len);

  Serial.println("");
  Serial.print(F("Set coding rate: ")); Serial.println(cr);
  int16_t state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->setCodingRate(cr);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag); 
  }
  else
  {
    state = ((SX1268*)lora)->setCodingRate(cr);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }
    
  readState(state);

  if (state == ERR_NONE)
    status.modeminfo.cr = cr;

  return state;
}

int16_t Radio::remote_crc(char* payload, size_t payload_len)
{
  bool crc = _atoi(payload, payload_len);

  Serial.println("");
  Serial.print(F("Set CRC "));  if (crc) Serial.println(F("ON")); else Serial.println(F("OFF"));
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->setCRC(crc);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag); 
  }
  else
  {
    state = ((SX1268*)lora)->setCRC(crc);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  readState(state);
  return state;
}

int16_t Radio::remote_lsw(char* payload, size_t payload_len)
{
  uint8_t sw = _atoi(payload, payload_len);

  Serial.println("");
  Serial.print(F(" 0x"));Serial.print(sw,HEX);
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    state = ((SX1278*)lora)->setSyncWord(sw);
  else
    state = ((SX1268*)lora)->setSyncWord(sw, 0x44);

  readState(state);
  return state;
}

int16_t Radio::remote_fldro(char* payload, size_t payload_len)
{
  bool ldro = _atoi(payload, payload_len);

  Serial.println("");
  Serial.print(F("Set ForceLDRO "));  if (ldro) Serial.println(F("ON")); else Serial.println(F("OFF"));
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->forceLDRO(ldro);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag); 
  }
  else
  {
    state = ((SX1268*)lora)->forceLDRO(ldro);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  readState(state);

  if (state == ERR_NONE) {
    if (ldro)
      status.modeminfo.fldro=true; 
    else 
      status.modeminfo.fldro=false;
  }

  return state;
}

int16_t Radio::remote_aldro(char* payload, size_t payload_len)
{
  Serial.println("");
  Serial.print(F("Set AutoLDRO ")); 
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->autoLDRO();
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag); 
  }
  else
  {
    state = ((SX1268*)lora)->autoLDRO();
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  readState(state);
  return state;
}

int16_t Radio::remote_pl(char* payload, size_t payload_len)
{
  uint16_t pl = _atoi(payload, payload_len);
  Serial.println("");
  Serial.print(F("Set Preamble ")); Serial.println(pl);
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->setPreambleLength(pl);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag); 
  }
  else
  {
    state = ((SX1268*)lora)->setPreambleLength(pl);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  readState(state);
  if (state == ERR_NONE)
    status.modeminfo.preambleLength = pl;

  return state;
}

int16_t Radio::remote_begin_lora(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload, payload_len);
  float   freq = doc[0];
  float   bw  =  doc[1];
  uint8_t sf  =  doc[2];
  uint8_t cr  =  doc[3];
  uint8_t syncWord78 =  doc[4];
  int8_t  power = doc[5];
  uint8_t current_limit = doc[6];
  uint16_t preambleLength = doc[7];
  uint8_t gain = doc[8];
  uint16_t syncWord68 =  doc[4];

  Serial.println("");
  Serial.print(F("Set Frequency: ")); Serial.print(freq, 3);Serial.println(F(" MHz"));
  Serial.print(F("Set bandwidth: ")); Serial.print(bw, 3);Serial.println(F(" kHz"));
  Serial.print(F("Set spreading factor: ")); Serial.println(sf);
  Serial.print(F("Set coding rate: ")); Serial.println(cr);
  Serial.print(F("Set sync Word 127x: 0x")); Serial.println(syncWord78,HEX);
  Serial.print(F("Set sync Word 126x: 0x")); Serial.println(syncWord68,HEX);
  Serial.print(F("Set Power: ")); Serial.println(power);
  Serial.print(F("Set C limit: ")); Serial.println(current_limit);
  Serial.print(F("Set Preamble: ")); Serial.println(preambleLength);
  Serial.print(F("Set Gain: ")); Serial.println(gain);
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    ((SX1278*)lora)->sleep();   // sleep mandatory if FastHop isn't ON.
    state = ((SX1278*)lora)->begin(freq, bw, sf, cr, syncWord78, power, preambleLength, gain);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag);
  }
  else
  {
    state = ((SX1268*)lora)->begin(freq, bw, sf, cr, syncWord68, power, preambleLength, ConfigManager::getInstance().getBoardConfig().L_TCXO_V);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }
  
  readState(state);
  if (state == ERR_NONE)
  {
    status.modeminfo.modem_mode = "LoRa";
    status.modeminfo.frequency  = freq;
    status.modeminfo.bw         = bw;
    status.modeminfo.power      = power ;
    status.modeminfo.preambleLength = preambleLength;
    status.modeminfo.sf         = sf;
    status.modeminfo.cr         = cr;
  }

  return state;
}

int16_t Radio::remote_begin_fsk(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload, payload_len);
  float   freq  = doc[0];
  float  	br    = doc[1];
  float   freqDev  =  doc[2];
  float   rxBw  =  doc[3];
  int8_t  power = doc[4];
  uint8_t currentlimit = doc[5];
  uint16_t preambleLength = doc[6];
  bool    enableOOK = doc[7];
  uint8_t   dataShaping = doc[8];

  Serial.println("");
  Serial.print(F("Set Frequency: ")); Serial.print(freq, 3);Serial.println(F(" MHz"));
  Serial.print(F("Set bit rate: ")); Serial.print(br, 3);Serial.println(F(" kbps"));
  Serial.print(F("Set Frequency deviation: ")); Serial.print(freqDev, 3);Serial.println(F(" kHz"));
  Serial.print(F("Set receiver bandwidth: ")); Serial.print(rxBw, 3);Serial.println(F(" kHz"));
  Serial.print(F("Set Power: ")); Serial.println(power);
  Serial.print(F("Set Current limit: ")); Serial.println(currentlimit);
  Serial.print(F("Set Preamble Length: ")); Serial.println(preambleLength);
  Serial.print(F("OOK Modulation "));  if (enableOOK) Serial.println(F("ON")); else Serial.println(F("OFF"));
  Serial.print(F("Set datashaping ")); Serial.println(dataShaping);

  int16_t state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X) {
    state = ((SX1278*)lora)->beginFSK(freq, br, freqDev, rxBw, power, preambleLength, enableOOK);
    ((SX1278*)lora)->setDataShaping(dataShaping);
    ((SX1278*)lora)->startReceive();
    ((SX1278*)lora)->setDio0Action(setFlag);

  } else {
    state = ((SX1268*)lora)->beginFSK(freq, br, freqDev, rxBw, power, preambleLength, ConfigManager::getInstance().getBoardConfig().L_TCXO_V);
    ((SX1268*)lora)->setDataShaping(dataShaping);
    ((SX1268*)lora)->startReceive();
    ((SX1268*)lora)->setDio1Action(setFlag);
  }
  readState(state);
  
  if (state == ERR_NONE)
  {
    status.modeminfo.modem_mode = "FSK";
    status.modeminfo.frequency  = freq;
    status.modeminfo.rxBw       = rxBw;
    status.modeminfo.power      = power ;
    status.modeminfo.preambleLength = preambleLength;
    status.modeminfo.bitrate    = br;
    status.modeminfo.freqDev    = freqDev;
    status.modeminfo.dataShaping= dataShaping;
  }

  return state;
}

int16_t Radio::remote_br(char* payload, size_t payload_len)
{
  uint8_t br = _atoi(payload, payload_len);
  Serial.println("");
  Serial.print(F("Set FSK Bit rate: ")); Serial.println(br);
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    state = ((SX1278*)lora)->setBitRate(br);
  else
    state = ((SX1268*)lora)->setBitRate(br);

  readState(state);
  if (state == ERR_NONE)
    status.modeminfo.bitrate = br;
  
  return state;
}

int16_t Radio::remote_fd(char* payload, size_t payload_len)
{
  uint8_t fd = _atoi(payload, payload_len);
  Serial.println("");
  Serial.print(F("Set FSK Frequency Des. : ")); Serial.println(fd);

  int16_t state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    state = ((SX1278*)lora)->setFrequencyDeviation(fd);
  else
    state = ((SX1268*)lora)->setFrequencyDeviation(fd);

  readState(state);
   if (state == ERR_NONE)
    status.modeminfo.freqDev = fd;

  return state;
}

int16_t Radio::remote_fbw(char* payload, size_t payload_len)
{
  float frequency = _atof(payload, payload_len);
  Serial.println("");
  Serial.print(F("Set FSK bandwidth: ")); Serial.print(frequency, 3); Serial.println(F(" kHz"));

  int16_t state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    state = ((SX1278*)lora)->setRxBandwidth(frequency);
  else
    state = ((SX1268*)lora)->setRxBandwidth(frequency);

  readState(state);
  if (state == ERR_NONE)
    status.modeminfo.rxBw  = frequency;
  
  return state;
}

int16_t Radio::remote_fsw(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload, payload_len);
  uint8_t syncWord[7];
  uint8_t synnwordsize = doc[0];

  Serial.println("");
  Serial.print(F("Set SyncWord Size ")); Serial.print(synnwordsize); Serial.print(F("-> "));

  for (uint8_t words=0; words<synnwordsize;words++)
  {
    syncWord[words]=doc[words+1];
    Serial.print(F(" 0x"));Serial.print(syncWord[words],HEX);Serial.print(F(", "));
  }

  int16_t state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    state = ((SX1278*)lora)->setSyncWord(syncWord, synnwordsize);
  else
    state = ((SX1268*)lora)->setSyncWord(syncWord, synnwordsize);

  readState(state);
  return state;
}

int16_t Radio::remote_fook(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(60);
  deserializeJson(doc, payload, payload_len);
  bool    enableOOK = doc[0];
  uint8_t ook_shape = doc[1];

  Serial.println("");
  Serial.print(F("OOK Modulation "));  if (enableOOK) Serial.println(F("ON")); else Serial.println(F("OFF"));
  Serial.print(F("Set OOK datashaping ")); Serial.println(ook_shape);
  int state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
  {
    state = ((SX1278*)lora)->setOOK(enableOOK);
  }
  else
  {
    Serial.println(F("OOK not supported by the selected lora module!"));
    return -1;
  }

  readState(state);

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setDataShapingOOK(ook_shape);

  readState(state);
  return state;
}

void Radio::remote_SPIwriteRegister(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(60);
  deserializeJson(doc, payload, payload_len);
  uint8_t     reg = doc[0];
  uint8_t     data = doc[1];
  Serial.println("");

  Serial.print(F("REG ID: 0x"));
  Serial.println(reg, HEX);
  Serial.print(F("to : 0x"));
  Serial.println(data, HEX);

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    ((SX1278*)lora)->_mod->SPIwriteRegister(reg,data);
  else
    ((SX1268*)lora)->_mod->SPIwriteRegister(reg,data);
}

int16_t Radio::remote_SPIreadRegister(char* payload, size_t payload_len)
{
  uint8_t     reg = _atoi(payload, payload_len);
  uint8_t     data = 0 ;
  Serial.println("");

  Serial.print(F("REG ID: 0x"));
  Serial.print(reg, HEX);
  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    data = ((SX1278*)lora)->_mod->SPIreadRegister(reg);
  else
    data = ((SX1268*)lora)->_mod->SPIreadRegister(reg);

  Serial.print(F(" HEX : 0x"));
  Serial.print(data, HEX);
  Serial.print(F(" BIN : "));
  Serial.println(data, BIN);
  
  readState(state);
  return data;
}

int16_t Radio::remote_SPIsetRegValue(char* payload, size_t payload_len)
{
  DynamicJsonDocument doc(120);
  deserializeJson(doc, payload, payload_len);
  uint8_t reg = doc[0];
  uint8_t value = doc[1];
  uint8_t msb = doc[2];
  uint8_t lsb = doc[3];
  uint8_t checkinterval = doc[4];
  
  Serial.println("");
  Serial.print(F("REG ID: 0x"));
  Serial.println(reg, HEX);
  Serial.print(F("to HEX: 0x"));
  Serial.print(value, HEX);
  Serial.print(F("bin : 0x "));
  Serial.println(value, BIN);
  Serial.print(F("msb : "));
  Serial.println(msb);
  Serial.print(F("lsb : "));
  Serial.println(lsb);
  Serial.print(F("check_interval : "));
  Serial.println(checkinterval);

  int16_t state = 0;

  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
    state = ((SX1278*)lora)->_mod->SPIsetRegValue(reg, value, msb, lsb, checkinterval);
  else
    state = ((SX1268*)lora)->_mod->SPIsetRegValue(reg, value, msb, lsb, checkinterval);
  
  readState(state);
  return state;
}

double Radio::_atof(const char* buff, size_t length)
{
  char* str = new char[length+1];
  memcpy(str, buff, length);
  str[length] = '\n';
  return atof(str);
}

int Radio::_atoi(const char* buff, size_t length)
{
  char* str = new char[length+1];
  memcpy(str, buff, length);
  str[length] = '\n';
  return atoi(str);
}
