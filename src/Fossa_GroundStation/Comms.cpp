#include "Comms.h"

int16_t FCP_Get_Frame_Length(char* callsign, uint8_t optDataLen, const char* password) {
  // check callsign
  if(callsign == NULL) {
    return(ERR_CALLSIGN_INVALID);
  }

  // callsign and function ID fields are always present
  int16_t frameLen = strlen(callsign) + 1;

  // optDataLen and optData might be present
  if(optDataLen > 0) {
    frameLen += 1 + optDataLen;
  }

  // check if the frame is encrpyted
  if(password != NULL) {
    frameLen += strlen(password);
    if(optDataLen == 0) {
      frameLen++;
    }
    frameLen += 16 - ((1 + optDataLen + strlen(password)) % 16);
  }

  return(frameLen);
}

int16_t FCP_Get_OptData_Length(char* callsign, uint8_t* frame, uint8_t frameLen, const uint8_t* key, const char* password) {
  // check callsign
  if(callsign == NULL) {
    return(ERR_CALLSIGN_INVALID);
  }

  // check frame buffer
  if(frame == NULL) {
    return(ERR_FRAME_INVALID);
  }

  // check frame length
  if(frameLen < strlen(callsign) + 1) {
    return(ERR_FRAME_INVALID);
  } else if(frameLen == strlen(callsign) + 1) {
    return(0);
  }

  // check encryption
  if((key != NULL) && (password != NULL)) {
    // encrypted frame

    // extract encrypted section
    uint8_t encSectionLen = frameLen - strlen(callsign) - 1;
    uint8_t* encSection = new uint8_t[encSectionLen];
    memcpy(encSection, frame + strlen(callsign) + 1, encSectionLen);

    // decrypt
//    aes128_dec_multiple(key, encSection, encSectionLen);

    // check password
    if(memcmp(encSection + 1, password, strlen(password)) == 0) {
      // check passed
      int16_t optDataLen = encSection[0] - strlen(password);

      // deallocate memory
      delete[] encSection;
      return(optDataLen);
    }

    // deallocate memory
    delete[] encSection;

  } else {
    // unencrypted frame
    int16_t optDataLen = frame[strlen(callsign) + 1];

    // check if optDataLen field matches the expected length
    if(optDataLen != (uint8_t)(frameLen - strlen(callsign) - 2)) {
      // length mismatch
      return(ERR_LENGTH_MISMATCH);
    }

    return(optDataLen);
  }

  return(ERR_INCORRECT_PASSWORD);
}

int16_t FCP_Get_FunctionID(char* callsign, uint8_t* frame, uint8_t frameLen) {
  // check callsign
  if(callsign == NULL) {
    return(ERR_CALLSIGN_INVALID);
  }

  // check frame buffer
  if(frame == NULL) {
    return(ERR_FRAME_INVALID);
  }

  // check frame length
  if(frameLen < strlen(callsign) + 1) {
    return(ERR_FRAME_INVALID);
  }

  return((int16_t)frame[strlen(callsign)]);
}

int16_t FCP_Get_OptData(char* callsign, uint8_t* frame, uint8_t frameLen, uint8_t* optData, const uint8_t* key, const char* password) {
  // check callsign
  if(callsign == NULL) {
    return(ERR_CALLSIGN_INVALID);
  }

  // check frame
  if(frame == NULL) {
    return(ERR_FRAME_INVALID);
  }

  // get frame pointer
  uint8_t* framePtr = frame;

  // check callsign
  if(memcmp(framePtr, callsign, strlen(callsign)) != 0) {
    // incorrect callsign
    return(ERR_CALLSIGN_INVALID);
  }
  framePtr += strlen(callsign);

  // skip function ID
  framePtr += 1;

  // check encryption
  if((key != NULL) && (password != NULL)) {
    // encrypted frame

    // extract encrypted section
    uint8_t encSectionLen = frameLen - strlen(callsign) - 1;
    uint8_t* encSection = new uint8_t[encSectionLen];
    memcpy(encSection, framePtr, encSectionLen);

    // decrypt
//    aes128_dec_multiple(key, encSection, encSectionLen);
    uint8_t* encSectionPtr = encSection;

    // get optional data length
    uint8_t optDataLen = *encSectionPtr;
    encSectionPtr += 1;

    // check password
    if(memcmp(encSectionPtr, password, strlen(password)) == 0) {
      // check passed

      // get optional data
      encSectionPtr += strlen(password);
      memcpy(optData, encSectionPtr, optDataLen - strlen(password));

      // deallocate memory
      delete[] encSection;
      return(ERR_NONE);
    }

    // deallocate memory
    delete[] encSection;

  } else {
    // unencrypted frame

    // get optional data (if present)
    if(frameLen > strlen(callsign) + 1) {
      if(optData == NULL) {
        return(ERR_FRAME_INVALID);
      }

      // get option data length
      uint8_t optDataLen = *framePtr;
      framePtr += 1;

      // get optional data
      memcpy(optData, framePtr, optDataLen);
      framePtr += optDataLen;
    }

    return(ERR_NONE);
  }

  return(ERR_INCORRECT_PASSWORD);
}

int16_t FCP_Encode(uint8_t* frame, char* callsign, uint8_t functionId, uint8_t optDataLen, uint8_t* optData, const uint8_t* key, const char* password) {
  // check callsign
  if(callsign == NULL) {
    return(ERR_CALLSIGN_INVALID);
  }

  // check some optional data are provided when optional data length is not 0 (and vice versa)
  if(((optDataLen > 0) && (optData == NULL)) || ((optData != NULL) && (optDataLen == 0))) {
    return(ERR_FRAME_INVALID);
  }

  // check frame buffer
  if(frame == NULL) {
    return(ERR_FRAME_INVALID);
  }

  // get frame pointer
  uint8_t* framePtr = frame;

  // set callsign
  memcpy(framePtr, callsign, strlen(callsign));
  framePtr += strlen(callsign);

  // set function ID
  *framePtr = functionId;
  framePtr += 1;

  // check encryption
  if((key != NULL) && (password != NULL)) {
    // encrypted frame

    // create encrypted section
    uint8_t encSectionLen = 1 + strlen(password) + optDataLen;
    uint8_t paddingLen = 16 - (encSectionLen % 16);
    uint8_t* encSection = new uint8_t[encSectionLen + paddingLen];
    uint8_t* encSectionPtr = encSection;

    // set optional data length
    *encSectionPtr = optDataLen + strlen(password);
    encSectionPtr += 1;

    // set password
    memcpy(encSectionPtr, password, strlen(password));
    encSectionPtr += strlen(password);

    // set optional data
    if(optData != NULL) {
      memcpy(encSectionPtr, optData, optDataLen);
      encSectionPtr += optDataLen;
    }

    // add padding
    for(uint8_t i = 0; i < paddingLen; i++) {
      *(encSectionPtr + i) = (uint8_t)random(0x00, 0x100);
    }

    // encrypt
//    aes128_enc_multiple(key, encSection, encSectionLen + paddingLen);

    // copy encrypted section into frame buffer
    memcpy(framePtr, encSection, encSectionLen + paddingLen);

    // deallocate memory
    delete[] encSection;
    return(ERR_NONE);

  } else {
    // unencrypted frame

    // check option data presence
    if(optDataLen > 0) {
      // set optional data length
      *framePtr = optDataLen;
      framePtr += 1;

      // set optional data
      memcpy(framePtr, optData, optDataLen);
      framePtr += optDataLen;
    }

    return(ERR_NONE);
  }

  return(ERR_FRAME_INVALID);
}

float FCP_Get_Battery_Charging_Voltage(uint8_t* optData) {
  return(FCP_System_Info_Get_Voltage(optData, 0));
}

float FCP_Get_Battery_Charging_Current(uint8_t* optData) {
  if(optData == NULL) {
    return(0);
  }

  int16_t raw;
  memcpy(&raw, optData + 1, sizeof(int16_t));
  return((float)raw * ((float)CURRENT_MULTIPLIER / (float)CURRENT_UNIT));
}

float FCP_Get_Battery_Voltage(uint8_t* optData) {
  return(FCP_System_Info_Get_Voltage(optData, 3));
}

float FCP_Get_Solar_Cell_Voltage(uint8_t cell, uint8_t* optData) {
  if(cell > 2) {
    return(0);
  }

  return(FCP_System_Info_Get_Voltage(optData, 4 + cell));
}

float FCP_Get_Battery_Temperature(uint8_t* optData) {
  return(FCP_System_Info_Get_Temperature(optData, 7));
}

float FCP_Get_Board_Temperature(uint8_t* optData) {
  return(FCP_System_Info_Get_Temperature(optData, 9));
}

int8_t FCP_Get_MCU_Temperature(uint8_t* optData) {
  if(optData == NULL) {
    return(0);
  }

  int8_t val;
  memcpy(&val, optData + 10, sizeof(int8_t));
  return(val);
}

uint16_t FCP_Get_Reset_Counter(uint8_t* optData) {
  if(optData == NULL) {
    return(0);
  }

  uint16_t val;
  memcpy(&val, optData + 12, sizeof(uint16_t));
  return(val);
}

uint8_t FCP_Get_Power_Configuration(uint8_t* optData) {
  if(optData == NULL) {
    return(0);
  }

  return(optData[14]);
}

float FCP_System_Info_Get_Voltage(uint8_t* optData, uint8_t pos) {
  if(optData == NULL) {
    return(0);
  }

  return((float)optData[pos] * ((float)VOLTAGE_MULTIPLIER / (float)VOLTAGE_UNIT));
}

float FCP_System_Info_Get_Temperature(uint8_t* optData, uint8_t pos) {
  if(optData == NULL) {
    return(0);
  }

  int16_t raw;
  memcpy(&raw, optData + pos, sizeof(int16_t));
  return((float)raw * ((float)TEMPERATURE_MULTIPLIER / (float)TEMPERATURE_UNIT));
}
