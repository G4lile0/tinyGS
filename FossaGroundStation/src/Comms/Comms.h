#ifndef _FOSSA_COMMS_H
#define _FOSSA_COMMS_H

//#include <aes.h>
#include <string.h>

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #error "Unsupported Arduino version (< 1.0.0)"
#endif

// version definitions
#define FCP_VERSION_MAJOR  (0x01)
#define FCP_VERSION_MINOR  (0x00)
#define FCP_VERSION_PATCH  (0x00)
#define FCP_VERSION_EXTRA  (0x00)

#define FCP_VERSION ((FCP_VERSION_MAJOR << 24) | (FCP_VERSION_MINOR << 16) | (FCP_VERSION_PATCH << 8) | (FCP_VERSION_EXTRA))

// constants
#define VOLTAGE_MULTIPLIER                            20 // 20 mV resolution
#define VOLTAGE_UNIT                                  1000
#define CURRENT_MULTIPLIER                            10 // 10 uA resolution
#define CURRENT_UNIT                                  1000000
#define TEMPERATURE_MULTIPLIER                        10 // 0.01 deg C resolution
#define TEMPERATURE_UNIT                              1000

// status codes
#define ERR_NONE                                      0
#define ERR_CALLSIGN_INVALID                          -1
#define ERR_FRAME_INVALID                             -2
#define ERR_INCORRECT_PASSWORD                        -3
#define ERR_LENGTH_MISMATCH                           -4

// communication protocol definitions
#define RESPONSE_OFFSET                                 0x20
#define PRIVATE_OFFSET                                  0x40

// public commands (unencrypted uplink messages)
#define CMD_PING                                        (0x00)
#define CMD_RETRANSMIT                                  (0x01)
#define CMD_RETRANSMIT_CUSTOM                           (0x02)
#define CMD_TRANSMIT_SYSTEM_INFO                        (0x03)
#define CMD_GET_PACKET_INFO                             (0x04)
#define CMD_GET_STATISTICS                              (0x05)
#define CMD_GET_FULL_SYSTEM_INFO                        (0x06)
#define CMD_STORE_AND_FORWARD_ADD                       (0x07)
#define CMD_STORE_AND_FORWARD_REQUEST                   (0x08)

#define NUM_PUBLIC_COMMANDS                             (9)

// public responses (unencrypted downlink messages)
#define RESP_PONG                                       (CMD_PING + RESPONSE_OFFSET)
#define RESP_REPEATED_MESSAGE                           (CMD_RETRANSMIT + RESPONSE_OFFSET)
#define RESP_REPEATED_MESSAGE_CUSTOM                    (CMD_RETRANSMIT_CUSTOM + RESPONSE_OFFSET)
#define RESP_SYSTEM_INFO                                (CMD_TRANSMIT_SYSTEM_INFO + RESPONSE_OFFSET)
#define RESP_PACKET_INFO                                (CMD_GET_PACKET_INFO + RESPONSE_OFFSET)
#define RESP_STATISTICS                                 (CMD_GET_STATISTICS + RESPONSE_OFFSET)
#define RESP_FULL_SYSTEM_INFO                           (CMD_GET_FULL_SYSTEM_INFO + RESPONSE_OFFSET)
#define RESP_STORE_AND_FORWARD_ASSIGNED_SLOT            (CMD_STORE_AND_FORWARD_ADD + RESPONSE_OFFSET)
#define RESP_FORWARDED_MESSAGE                          (CMD_STORE_AND_FORWARD_REQUEST + RESPONSE_OFFSET)
#define RESP_DEPLOYMENT_STATE                           (NUM_PUBLIC_COMMANDS + RESPONSE_OFFSET)
#define RESP_RECORDED_SOLAR_CELLS                       (NUM_PUBLIC_COMMANDS + 1 + RESPONSE_OFFSET)
#define RESP_CAMERA_STATE                               (NUM_PUBLIC_COMMANDS + 2 + RESPONSE_OFFSET)
#define RESP_RECORDED_IMU                               (NUM_PUBLIC_COMMANDS + 3 + RESPONSE_OFFSET)
#define RESP_ADCS_RESULT                                (NUM_PUBLIC_COMMANDS + 4 + RESPONSE_OFFSET)
#define RESP_GPS_LOG                                    (NUM_PUBLIC_COMMANDS + 5 + RESPONSE_OFFSET)
#define RESP_GPS_LOG_LENGTH                             (NUM_PUBLIC_COMMANDS + 6 + RESPONSE_OFFSET)
#define RESP_FLASH_CONTENTS                             (NUM_PUBLIC_COMMANDS + 7 + RESPONSE_OFFSET)
#define RESP_CAMERA_PICTURE                             (NUM_PUBLIC_COMMANDS + 8 + RESPONSE_OFFSET)
#define RESP_CAMERA_PICTURE_LENGTH                      (NUM_PUBLIC_COMMANDS + 9 + RESPONSE_OFFSET)

// private commands (encrypted uplink messages)
#define CMD_DEPLOY                                      (0x00 + PRIVATE_OFFSET)
#define CMD_RESTART                                     (0x01 + PRIVATE_OFFSET)
#define CMD_WIPE_EEPROM                                 (0x02 + PRIVATE_OFFSET)
#define CMD_SET_TRANSMIT_ENABLE                         (0x03 + PRIVATE_OFFSET)
#define CMD_SET_CALLSIGN                                (0x04 + PRIVATE_OFFSET)
#define CMD_SET_SF_MODE                                 (0x05 + PRIVATE_OFFSET)
#define CMD_SET_MPPT_MODE                               (0x06 + PRIVATE_OFFSET)
#define CMD_SET_LOW_POWER_ENABLE                        (0x07 + PRIVATE_OFFSET)
#define CMD_SET_RECEIVE_WINDOWS                         (0x08 + PRIVATE_OFFSET)
#define CMD_RECORD_SOLAR_CELLS                          (0x09 + PRIVATE_OFFSET)
#define CMD_CAMERA_CAPTURE                              (0x0A + PRIVATE_OFFSET)
#define CMD_SET_POWER_LIMITS                            (0x0B + PRIVATE_OFFSET)
#define CMD_SET_RTC                                     (0x0C + PRIVATE_OFFSET)
#define CMD_RECORD_IMU                                  (0x0D + PRIVATE_OFFSET)
#define CMD_RUN_ADCS                                    (0x0E + PRIVATE_OFFSET)
#define CMD_LOG_GPS                                     (0x0F + PRIVATE_OFFSET)
#define CMD_GET_GPS_LOG                                 (0x10 + PRIVATE_OFFSET)
#define CMD_GET_FLASH_CONTENTS                          (0x11 + PRIVATE_OFFSET)
#define CMD_GET_PICTURE_LENGTH                          (0x12 + PRIVATE_OFFSET)
#define CMD_GET_PICTURE_BURST                           (0x13 + PRIVATE_OFFSET)

#define NUM_PRIVATE_COMMANDS                            (19)

#define PRINT_BUFF(BUFF, LEN) { \
  for(size_t i = 0; i < LEN; i++) { \
    Serial.print(F("0x")); \
    Serial.print(BUFF[i], HEX); \
    Serial.print('\t'); \
    Serial.write(BUFF[i]); \
    Serial.println(); \
  } }

int16_t FCP_Get_Frame_Length(char* callsign, uint8_t optDataLen = 0, const char* password = NULL);
int16_t FCP_Get_FunctionID(char* callsign, uint8_t* frame, uint8_t frameLen);
int16_t FCP_Get_OptData_Length(char* callsign, uint8_t* frame, uint8_t frameLen, const uint8_t* key = NULL, const char* password = NULL);
int16_t FCP_Get_OptData(char* callsign, uint8_t* frame, uint8_t frameLen, uint8_t* optData, const uint8_t* key = NULL, const char* password = NULL);

int16_t FCP_Encode(uint8_t* frame, char* callsign, uint8_t functionId, uint8_t optDataLen = 0, uint8_t* optData = NULL, const uint8_t* key = NULL, const char* password = NULL);

float FCP_Get_Battery_Voltage(uint8_t* optData);
float FCP_Get_Battery_Charging_Current(uint8_t* optData);
float FCP_Get_Battery_Charging_Voltage(uint8_t* optData);
uint32_t FCP_Get_Uptime_Counter(uint8_t* optData);
uint8_t FCP_Get_Power_Configuration(uint8_t* optData);
uint16_t FCP_Get_Reset_Counter(uint8_t* optData);
float FCP_Get_Solar_Cell_Voltage(uint8_t cell, uint8_t* optData);
float FCP_Get_Battery_Temperature(uint8_t* optData);
float FCP_Get_Board_Temperature(uint8_t* optData);
int8_t FCP_Get_MCU_Temperature(uint8_t* optData);

float FCP_System_Info_Get_Voltage(uint8_t* optData, uint8_t pos);
float FCP_System_Info_Get_Temperature(uint8_t* optData, uint8_t pos);
float FCP_System_Info_Get_Current(uint8_t* optData, uint8_t pos);

#endif