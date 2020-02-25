/*
  Radio.cpp - Class to handle radio communications
  
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

#include "Radio.h"
#include "ArduinoJson.h"
#include "../Comms/Comms.h"
#include <base64.h>

char callsign[] = "FOSSASAT-1";
bool received = false;
bool eInterrupt = true;

// modem configuration
#define LORA_CARRIER_FREQUENCY        436.7f  // MHz
#define LORA_BANDWIDTH                125.0f  // kHz dual sideband
#define LORA_SPREADING_FACTOR         11
#define LORA_SPREADING_FACTOR_ALT     10
#define LORA_CODING_RATE              8       // 4/8, Extended Hamming
#define LORA_OUTPUT_POWER             20      // dBm
#define LORA_CURRENT_LIMIT_7X         120     // mA
#define LORA_CURRENT_LIMIT_6X         120.0f     // mA
#define SYNC_WORD                     0x12    // sync word 
#define LORA_PREAMBLE_LENGTH          8U

Radio::Radio()
: spi(VSPI)
{
  
}

void Radio::init(){
  Serial.print(F("[SX12xx] Initializing ... "));
  board_type board = ConfigManager::getInstance().getBoardConfig();
  
  spi.begin(board.L_SCK, board.L_MISO, board.L_MOSI, board.L_NSS);
  int state = 0;
  if (board.L_SX127X) {
    lora = new SX1278(new Module(board.L_NSS, board.L_DI00, board.L_DI01, spi));
    state = ((SX1278*)lora)->begin(LORA_CARRIER_FREQUENCY,
                                      LORA_BANDWIDTH,
                                      LORA_SPREADING_FACTOR,
                                      LORA_CODING_RATE,
                                      SYNC_WORD,
                                      LORA_OUTPUT_POWER,
                                      (uint8_t)LORA_CURRENT_LIMIT_7X);
  }
  else {
    lora = new SX1268(new Module(board.L_NSS, board.L_DI01, board.L_RST, board.L_BUSSY, spi));
    state = ((SX1268*)lora)->begin(LORA_CARRIER_FREQUENCY,
                                      LORA_BANDWIDTH,
                                      LORA_SPREADING_FACTOR,
                                      LORA_CODING_RATE,
                                      SYNC_WORD,
                                      LORA_OUTPUT_POWER,
                                      LORA_CURRENT_LIMIT_6X,
                                      LORA_PREAMBLE_LENGTH,
                                      board.L_TCXO_V);
  }
  
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }

  // set the function that will be called
  // when new packet is received
  // attach the ISR to radio interrupt
  
  if (board.L_SX127X) {
    ((SX1278*)lora)->setDio0Action(setFlag);
  }
  else {
    ((SX1268*)lora)->setDio1Action(setFlag);
  }

  // start listening for LoRa packets
  Serial.print(F("[SX12x8] Starting to listen ... "));
  if (board.L_SX127X)
    state = ((SX1278*)lora)->startReceive();
  else
    state = ((SX1268*)lora)->startReceive();

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    Serial.println(String("Go to the config panel (") + WiFi.localIP().toString() + ") and check if the board selected matches your hardware.");
    return;
  }

  ready = true;
}

void Radio::setFlag() {
  if(!eInterrupt) {
    return;
  }
  received = true;
}

void Radio::enableInterrupt() {
  eInterrupt = true;
}

void Radio::disableInterrupt() {
  eInterrupt = false;
}

int Radio::sendFrame(uint8_t functionId, const char* data) {
  // build frame
  uint8_t optDataLen = strlen(data);

  // build frame
  uint8_t len = FCP_Get_Frame_Length(callsign, optDataLen);
  uint8_t* frame = new uint8_t[len];
  if (optDataLen > 0)
    FCP_Encode(frame, callsign, functionId, optDataLen, (uint8_t*)data);
  else
    FCP_Encode(frame, callsign, functionId);

  // send data
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X) {
    SX1278* l = (SX1278*)lora;
    state = l->transmit(frame, len);
    l->setDio0Action(setFlag);
    l->startReceive();
  }
  else {
    SX1268* l = (SX1268*)lora;
    state = l->transmit(frame, len);
    l->setDio1Action(setFlag);
    l->startReceive();
  }
  delete[] frame;

  return state;
}


void Radio::sendPing() {
  Serial.print(F("Sending ping frame ... "));
  int state = sendFrame(CMD_PING);

  // check transmission success
  if (state == ERR_NONE) {
    Serial.println(F("sent successfully!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    Serial.println(String("Go to the config panel (") + WiFi.localIP().toString() + ") and check if the board selected matches your hardware.");
  }
}

void Radio::requestInfo() {
  Serial.print(F("Requesting system info ... "));
  int state = sendFrame(CMD_TRANSMIT_SYSTEM_INFO);
  
  // check transmission success
  if (state == ERR_NONE) {
    Serial.println(F("sent successfully!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    Serial.println(String("Go to the config panel (") + WiFi.localIP().toString() + ") and check if the board selected matches your hardware.");
  }
}

void Radio::requestPacketInfo() {
  Serial.print(F("Requesting last packet info ... "));
  int state = sendFrame(CMD_GET_PACKET_INFO);
  
  // check transmission success
  if (state == ERR_NONE) {
    Serial.println(F("sent successfully!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    Serial.println(String("Go to the config panel (") + WiFi.localIP().toString() + ") and check if the board selected matches your hardware.");
  }
}

void Radio::requestRetransmit(char* data) {
  Serial.print(F("Requesting retransmission ... "));
  int state = sendFrame(CMD_RETRANSMIT, data);
  // check transmission success
  if (state == ERR_NONE) {
    Serial.println(F("sent successfully!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    Serial.println(String("Go to the config panel (") + WiFi.localIP().toString() + ") and check if the board selected matches your hardware.");
  }
}

uint8_t Radio::listen() {
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
  // read received data
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X) {
    SX1278* l = (SX1278*)lora;
    respLen = l->getPacketLength();
    respFrame = new uint8_t[respLen];
    state = l->readData(respFrame, respLen);
    status.lastPacketInfo.rssi = l->getRSSI();
    status.lastPacketInfo.snr = l->getSNR();
    status.lastPacketInfo.frequencyerror = l->getFrequencyError();
  }
  else {
    SX1268* l = (SX1268*)lora;
    respLen = l->getPacketLength();
    respFrame = new uint8_t[respLen];
    state = l->readData(respFrame, respLen);
    status.lastPacketInfo.rssi = l->getRSSI();
    status.lastPacketInfo.snr = l->getSNR();
    //status.lastPacketInfo.frequencyerror = l->getFrequencyError();
  }
  
  // get function ID
  uint8_t functionId = FCP_Get_FunctionID(callsign, respFrame, respLen);
  Serial.print(F("Function ID: 0x"));
  Serial.println(functionId, HEX);

  // check optional data
  uint8_t *respOptData = nullptr;
  uint8_t respOptDataLen = FCP_Get_OptData_Length(callsign, respFrame, respLen);
  Serial.print(F("Optional data ("));
  Serial.print(respOptDataLen);
  Serial.println(F(" bytes):"));
  if(respOptDataLen > 0) {
    // read optional data
    respOptData = new uint8_t[respOptDataLen];
    FCP_Get_OptData(callsign, respFrame, respLen, respOptData);
    PRINT_BUFF(respFrame, respLen);
  }

  String encoded = base64::encode(respFrame, respLen);
  MQTT_Client::getInstance().sendRawPacket(encoded);

  delete[] respFrame;

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    status.lastPacketInfo.time = "";
  }
  else {
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

  if (state == ERR_NONE) {
    processReceivedFrame(functionId, respOptData, respLen);
  }
  delete[] respOptData;

  if (state == ERR_NONE) {
    return 0;
  } else if (state == ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("[SX12x8] CRC error!"));
    return 2;
  } else {
    // some other error occurred
    Serial.print(F("[SX12x8] Failed, code "));
    Serial.println(state);
    return 3;
  }
}

void Radio::processReceivedFrame(uint8_t functionId, uint8_t *respOptData, size_t respLen) {
  switch(functionId) {
    case RESP_PONG:
      Serial.println(F("Pong!"));
      MQTT_Client::getInstance().sendPong();
      break;

    case RESP_SYSTEM_INFO:
      Serial.println(F("System info:"));

      Serial.print(F("batteryChargingVoltage = "));
      status.sysInfo.batteryChargingVoltage = FCP_Get_Battery_Charging_Voltage(respOptData);
      Serial.println(FCP_Get_Battery_Charging_Voltage(respOptData));
      
      Serial.print(F("batteryChargingCurrent = "));
      status.sysInfo.batteryChargingCurrent = (FCP_Get_Battery_Charging_Current(respOptData), 4);
      Serial.println(FCP_Get_Battery_Charging_Current(respOptData), 4);

      Serial.print(F("batteryVoltage = "));
      status.sysInfo.batteryVoltage=FCP_Get_Battery_Voltage(respOptData);
      Serial.println(FCP_Get_Battery_Voltage(respOptData));          

      Serial.print(F("solarCellAVoltage = "));
      status.sysInfo.solarCellAVoltage= FCP_Get_Solar_Cell_Voltage(0, respOptData);
      Serial.println(FCP_Get_Solar_Cell_Voltage(0, respOptData));

      Serial.print(F("solarCellBVoltage = "));
      status.sysInfo.solarCellBVoltage= FCP_Get_Solar_Cell_Voltage(1, respOptData);
      Serial.println(FCP_Get_Solar_Cell_Voltage(1, respOptData));

      Serial.print(F("solarCellCVoltage = "));
      status.sysInfo.solarCellCVoltage= FCP_Get_Solar_Cell_Voltage(2, respOptData);
      Serial.println(FCP_Get_Solar_Cell_Voltage(2, respOptData));

      Serial.print(F("batteryTemperature = "));
      status.sysInfo.batteryTemperature=FCP_Get_Battery_Temperature(respOptData);
      Serial.println(FCP_Get_Battery_Temperature(respOptData));

      Serial.print(F("boardTemperature = "));
      status.sysInfo.boardTemperature=FCP_Get_Board_Temperature(respOptData);
      Serial.println(FCP_Get_Board_Temperature(respOptData));

      Serial.print(F("mcuTemperature = "));
      status.sysInfo.mcuTemperature =FCP_Get_MCU_Temperature(respOptData);
      Serial.println(FCP_Get_MCU_Temperature(respOptData));

      Serial.print(F("resetCounter = "));
      status.sysInfo.resetCounter=FCP_Get_Reset_Counter(respOptData);
      Serial.println(FCP_Get_Reset_Counter(respOptData));

      Serial.print(F("powerConfig = 0b"));
      status.sysInfo.powerConfig=FCP_Get_Power_Configuration(respOptData);
      Serial.println(FCP_Get_Power_Configuration(respOptData), BIN);

      MQTT_Client::getInstance().sendSystemInfo();
      break;

    case RESP_PACKET_INFO:
      Serial.println(F("Last packet info:"));

      Serial.print(F("SNR = "));
      Serial.print(respOptData[0] / 4.0);
      Serial.println(F(" dB"));

      Serial.print(F("RSSI = "));
      Serial.print(respOptData[1] / -2.0);
      Serial.println(F(" dBm"));
      break;

    case RESP_REPEATED_MESSAGE:
      Serial.println(F("Got repeated message:"));
      Serial.println((char*)respOptData);
      MQTT_Client::getInstance().sendMessage((char*)respOptData,respLen);
      break;

    default:
      Serial.println(F("Unknown function ID!"));
      break;
  }
}

// remote

void Radio::remote_freq(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  float frequency = doc[0];
  Serial.println("");
  Serial.print(F("Set Frequency: ")); Serial.print(frequency, 3);Serial.println(F(" MHz"));
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setFrequency(frequency);
  else
      state = ((SX1268*)lora)->setFrequency(frequency);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}

void Radio::remote_bw(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  float frequency = doc[0];
  Serial.println("");
  Serial.print(F("Set bandwidth: ")); Serial.print(frequency, 3);Serial.println(F(" kHz"));
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setBandwidth(frequency);
  else
      state = ((SX1268*)lora)->setBandwidth(frequency);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}

void Radio::remote_sf(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  uint8_t sf = doc[0];
  Serial.println("");
  Serial.print(F("Set spreading factor: ")); Serial.println(sf);
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setSpreadingFactor(sf);
  else
      state = ((SX1268*)lora)->setSpreadingFactor(sf);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}


void Radio::remote_cr(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  uint8_t cr = doc[0];
  Serial.println("");
  Serial.print(F("Set coding rate: ")); Serial.println(cr);
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setCodingRate(cr);
  else
      state = ((SX1268*)lora)->setCodingRate(cr);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}


void Radio::remote_crc(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  bool crc = doc[0];
  Serial.println("");
  Serial.print(F("Set CRC "));  if (crc) Serial.println(F("ON")); else Serial.println(F("OFF"));
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setCRC (crc);
  else
      state = ((SX1268*)lora)->setCRC (crc);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}

void Radio::remote_pl(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  uint16_t pl = doc[0];
  Serial.println("");
  Serial.print(F("Set Preamble ")); Serial.println(pl);
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setPreambleLength(pl);
  else
      state = ((SX1268*)lora)->setPreambleLength(pl);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}

void Radio::remote_begin_lora(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(256);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
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
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X) {
    state = ((SX1278*)lora)->begin(freq,
                                     bw,
                                     sf,
                                     cr,
                                     syncWord78,
                                     power,
                                     current_limit,
                                     preambleLength);
  }
  else {
    state = ((SX1268*)lora)->begin(freq,
                                     bw,
                                     sf,
                                     cr,
                                     syncWord68,
                                     power,
                                     current_limit,
                                     preambleLength,
                                     ConfigManager::getInstance().getBoardConfig().L_TCXO_V);
  }
  
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}


void Radio::remote_begin_fsk(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(256);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  float   freq  = doc[0];
  float  	br    = doc[1];
  float   freqDev  =  doc[2];
  float   rxBw  =  doc[3];
  int8_t  power = doc[4];
  uint8_t currentlimit = doc[5];
  uint16_t preambleLength = doc[6];
  bool    enableOOK = doc[7];
  float   dataShaping = doc[8];

  Serial.println("");
  Serial.print(F("Set Frequency: ")); Serial.print(freq, 3);Serial.println(F(" MHz"));
  Serial.print(F("Set bit rate: ")); Serial.print(br, 3);Serial.println(F(" kbps"));
  Serial.print(F("Set Frequency deviatio: ")); Serial.print(freqDev, 3);Serial.println(F(" MHz"));
  Serial.print(F("Set receiver bandwidth: ")); Serial.print(rxBw, 3);Serial.println(F(" kHz"));
  Serial.print(F("Set Power: ")); Serial.println(power);
  Serial.print(F("Set Current limit: ")); Serial.println(currentlimit);
  Serial.print(F("Set Preamble Length: ")); Serial.println(preambleLength);
  Serial.print(F("OOK Modulation "));  if (enableOOK) Serial.println(F("ON")); else Serial.println(F("OFF"));
  Serial.print(F("Set Sx1268 datashaping ")); Serial.println(dataShaping);

  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X) {
    state = ((SX1278*)lora)->beginFSK(freq,
                                     br,
                                     freqDev,
                                     rxBw,                                     
                                     power,
                                     currentlimit,
                                     preambleLength);
  }
  else {
    state = ((SX1268*)lora)->beginFSK(freq,
                                     br,
                                     freqDev,
                                     rxBw,                                     
                                     power,
                                     currentlimit,
                                     preambleLength,
                                     dataShaping,
                                     ConfigManager::getInstance().getBoardConfig().L_TCXO_V);

  }
  
  if (state == ERR_NONE) {
    Serial.println(F("success FSK enable!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }

}


void Radio::remote_br(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  uint8_t br = doc[0];
  Serial.println("");
  Serial.print(F("Set FSK Bit rate: ")); Serial.println(br);
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setBitRate(br);
  else
      state = ((SX1268*)lora)->setBitRate(br);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}


void Radio::remote_fd(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  uint8_t fd = doc[0];
  Serial.println("");
  Serial.print(F("Set FSK Bit rate: ")); Serial.println(fd);
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setFrequencyDeviation(fd);
  else
      state = ((SX1268*)lora)->setFrequencyDeviation(fd);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}


void Radio::remote_fbw(char* payload, size_t payload_len) {
  DynamicJsonDocument doc(60);
  char payloadStr[payload_len+1];
  memcpy(payloadStr, payload, payload_len);
  payloadStr[payload_len] = '\0';
  deserializeJson(doc, payload);
  float frequency = doc[0];
  Serial.println("");
  Serial.print(F("Set FSK bandwidth: ")); Serial.print(frequency, 3);Serial.println(F(" kHz"));
  int state = 0;
  if (ConfigManager::getInstance().getBoardConfig().L_SX127X)
      state = ((SX1278*)lora)->setRxBandwidth(frequency);
  else
      state = ((SX1268*)lora)->setRxBandwidth(frequency);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
}






