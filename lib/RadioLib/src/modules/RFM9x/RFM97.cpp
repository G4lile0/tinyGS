#include "RFM97.h"
#if !defined(RADIOLIB_EXCLUDE_RFM9X)

RFM97::RFM97(Module* mod) : RFM95(mod) {

};

int16_t RFM97::setSpreadingFactor(uint8_t sf) {
  // check active modem
  if(getActiveModem() != RADIOLIB_SX127X_LORA) {
    return(RADIOLIB_ERR_WRONG_MODEM);
  }

  uint8_t newSpreadingFactor;

  // check allowed spreading factor values
  switch(sf) {
    case 6:
      newSpreadingFactor = RADIOLIB_SX127X_SF_6;
      break;
    case 7:
      newSpreadingFactor = RADIOLIB_SX127X_SF_7;
      break;
    case 8:
      newSpreadingFactor = RADIOLIB_SX127X_SF_8;
      break;
    case 9:
      newSpreadingFactor = RADIOLIB_SX127X_SF_9;
      break;
    default:
      return(RADIOLIB_ERR_INVALID_SPREADING_FACTOR);
  }

  // set spreading factor and if successful, save the new setting
  int16_t state = SX1278::setSpreadingFactorRaw(newSpreadingFactor);
  if(state == RADIOLIB_ERR_NONE) {
    SX127x::spreadingFactor = sf;
  }
  return(state);
}

#endif
