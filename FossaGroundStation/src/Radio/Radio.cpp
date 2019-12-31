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
#include "../Comms/Comms.h"

char callsign[] = "FOSSASAT-1";
bool received = false;
bool eInterrupt = true;

// modem configuration
#define LORA_CARRIER_FREQUENCY        436.7f  // MHz
#define LORA_BANDWIDTH                125.0f  // kHz dual sideband
#define LORA_SPREADING_FACTOR         11
#define LORA_SPREADING_FACTOR_ALT     10
#define LORA_CODING_RATE              8       // 4/8, Extended Hamming
#define LORA_OUTPUT_POWER             21      // dBm
#define LORA_CURRENT_LIMIT            120     // mA
#define SYNC_WORD_7X                  0xFF    // sync word when using SX127x
#define SYNC_WORD_6X                  0x0F0F  //                      SX126x

Radio::Radio(ConfigManager& x)
: configManager(x)
{
  
}

void Radio::init(){
  Serial.print(F("[SX12xx] Initializing ... "));
  board_type board = configManager.getBoardConfig();
  
  int state = 0;
  if (board.L_SX127X) {
    lora = new SX1278(new Module(board.L_NSS, board.L_DI00, board.L_DI01));
    state = ((SX1278*)lora)->begin(LORA_CARRIER_FREQUENCY,
                                      LORA_BANDWIDTH,
                                      LORA_SPREADING_FACTOR,
                                      LORA_CODING_RATE,
                                      SYNC_WORD_7X,
                                      17,
                                      (uint8_t)LORA_CURRENT_LIMIT);
  }
  else {
    lora = new SX1268(new Module(board.L_NSS, board.L_DI00, board.L_BUSSY));
    state = ((SX1268*)lora)->begin(LORA_CARRIER_FREQUENCY,
                                      LORA_BANDWIDTH,
                                      LORA_SPREADING_FACTOR,
                                      LORA_CODING_RATE,
                                      SYNC_WORD_6X,
                                      17,
                                      (uint8_t)LORA_CURRENT_LIMIT);
  }
  
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } 
  else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    delay(5000);
    ESP.restart();
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
    while (true);
  }
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
  if (configManager.getBoardConfig().L_SX127X) {
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
  }
}

void Radio::requestPacketInfo() {
  Serial.print(F("Requesting last packet info ... "));
  int state = sendFrame(CMD_GET_LAST_PACKET_INFO);
  
  // check transmission success
  if (state == ERR_NONE) {
    Serial.println(F("sent successfully!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
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
  }
}

uint8_t Radio::listen(uint8_t *&respOptData,  size_t &respLen, uint8_t &functionId){
  // check if the flag is set (received interruption)
  if(!received) 
    return 1;

  // disable the interrupt service routine while
  // processing the data
  disableInterrupt();

  // reset flag
  received = false;

  respLen = 0;
  uint8_t* respFrame = 0;
  int state = 0;
  // read received data
  if (configManager.getBoardConfig().L_SX127X) {
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
  functionId = FCP_Get_FunctionID(callsign, respFrame, respLen);
  Serial.print(F("Function ID: 0x"));
  Serial.println(functionId, HEX);

  // check optional data
  respOptData = nullptr;
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
  if (configManager.getBoardConfig().L_SX127X)
    ((SX1278*)lora)->startReceive();
  else
    ((SX1268*)lora)->startReceive();

  // we're ready to receive more packets,
  // enable interrupt service routine
  enableInterrupt();

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
