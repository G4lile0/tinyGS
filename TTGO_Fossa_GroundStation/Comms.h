#ifndef _FOSSA_COMMS_H
#define _FOSSA_COMMS_H

#include <Arduino.h>
//#include <AESLib.h>
//#include <aes.h>

#include <string.h>

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
#define RESPONSE_OFFSET                                 0x10
#define PRIVATE_OFFSET                                  0x20

// public commands (unencrypted uplink messages)
#define CMD_PING                                        (0x00)
#define CMD_RETRANSMIT                                  (0x01)
#define CMD_RETRANSMIT_CUSTOM                           (0x02)
#define CMD_TRANSMIT_SYSTEM_INFO                        (0x03)
#define CMD_GET_LAST_PACKET_INFO                        (0x04)

// public responses (unencrypted downlink messages)
#define RESP_PONG                                       (CMD_PING + RESPONSE_OFFSET)
#define RESP_REPEATED_MESSAGE                           (CMD_RETRANSMIT + RESPONSE_OFFSET)
#define RESP_REPEATED_MESSAGE_CUSTOM                    (CMD_RETRANSMIT_CUSTOM + RESPONSE_OFFSET)
#define RESP_SYSTEM_INFO                                (CMD_TRANSMIT_SYSTEM_INFO + RESPONSE_OFFSET)
#define RESP_LAST_PACKET_INFO                           (CMD_GET_LAST_PACKET_INFO + RESPONSE_OFFSET)

// private commands (encrypted uplink messages)
#define CMD_DEPLOY                                      (0x00 + PRIVATE_OFFSET)
#define CMD_RESTART                                     (0x01 + PRIVATE_OFFSET)
#define CMD_WIPE_EEPROM                                 (0x02 + PRIVATE_OFFSET)
#define CMD_SET_TRANSMIT_ENABLE                         (0x03 + PRIVATE_OFFSET)
#define CMD_SET_CALLSIGN                                (0x04 + PRIVATE_OFFSET)
#define CMD_SET_SF_MODE                                 (0x05 + PRIVATE_OFFSET)
#define CMD_SET_MPPT_MODE                               (0x06 + PRIVATE_OFFSET)
#define CMD_SET_LOW_POWER_ENABLE                        (0x07 + PRIVATE_OFFSET)

// private responses (encrypted downlink messages)
#define RESP_DEPLOYMENT_STATE                           (CMD_DEPLOY + RESPONSE_OFFSET)
#define RESP_INCORRECT_PASSWORD                         (CMD_RESTART + RESPONSE_OFFSET)

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

float FCP_Get_Battery_Charging_Voltage(uint8_t* optData);
float FCP_Get_Battery_Charging_Current(uint8_t* optData);
float FCP_Get_Battery_Voltage(uint8_t* optData);
float FCP_Get_Solar_Cell_Voltage(uint8_t cell, uint8_t* optData);
float FCP_Get_Battery_Temperature(uint8_t* optData);
float FCP_Get_Board_Temperature(uint8_t* optData);
uint16_t FCP_Get_Reset_Counter(uint8_t* optData);
int8_t FCP_Get_MCU_Temperature(uint8_t* optData);
uint8_t FCP_Get_Power_Configuration(uint8_t* optData);

float FCP_System_Info_Get_Voltage(uint8_t* optData, uint8_t pos);
float FCP_System_Info_Get_Temperature(uint8_t* optData, uint8_t pos);

#endif
