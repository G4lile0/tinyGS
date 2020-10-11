/*
  Status.h - Data structure for system status
  
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

#ifndef Status_h
#define Status_h

struct SysInfo {
  float batteryChargingVoltage = 0.0f;
  float batteryChargingCurrent = 0.0f;
  float batteryVoltage = 0.0f;
  float solarCellAVoltage = 0.0f;
  float solarCellBVoltage = 0.0f;
  float solarCellCVoltage = 0.0f;
  float batteryTemperature = 0.0f;
  float boardTemperature = 0.0f;
  int mcuTemperature = 0;
  int resetCounter = 0;
  char powerConfig = 0b11111111;
};

struct PacketInfo {
  String time = "Waiting      ";
  float rssi = 0;
  float snr = 0;
  float frequencyerror = 0;
};


struct ModemInfo {
  String satelite     = "Norbi (Norby)            ";
  String  modem_mode  = "LoRa" ;     // 1-LoRa  2-FSK  3-GMSK
  float   frequency   = 436.703; // MHz  
  float   bw          = 250.0; // kHz dual sideban
  uint8_t sf          =  10 ;
  uint8_t cr          =   5 ;
  int8_t  power       =  20 ;
  uint16_t preambleLength = 8;
  float  	bitrate    =  9.6 ;
  float   freqDev    =  5.0;
  float   rxBw       =  39.0;
  bool    enableOOK  =  0;
  float   dataShaping = 0.3;
};


struct Global_Frame_Text {
  uint8_t n;    
  uint8_t text_font;
  uint8_t text_alignment;
  int16_t text_pos_x;
  int16_t text_pos_y; 
  String  text = "123456789012345678901234567890";
};

struct Local_Frame_Text {
  uint8_t n;    
  uint8_t text_font;
  uint8_t text_alignment;
  int16_t text_pos_x;
  int16_t text_pos_y; 
  String  text = "123456789012345678901234567890";
};




struct Status {
  const uint32_t version = 2010111; // version year month day release
  bool mqtt_connected = false;
  SysInfo sysInfo;
  PacketInfo lastPacketInfo;
  ModemInfo modeminfo;
  float satPos[2] = {0, 0};
  uint8_t global_frame_text_leght = 0;
  uint8_t  local_frame_text_leght = 0;
  Global_Frame_Text global_frame_text[15];
  Local_Frame_Text   local_frame_text[15];
  
};



#endif