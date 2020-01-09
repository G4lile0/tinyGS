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
  String time = " Waiting      ";
  float rssi = 0;
  float snr = 0;
  float frequencyerror = 0;
};

struct Status {
  const uint32_t version = 2001081; // version year month day release
  bool mqtt_connected = false;
  SysInfo sysInfo;
  PacketInfo lastPacketInfo;
  float satPos[2] = {0, 0};
};

#endif