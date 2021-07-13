#include "Arduino.h"
#include <RadioLib.h>
#ifndef RADIOHAL_HPP
#define RADIOHAL_HPP


class IRadioHal {
public:
    virtual int16_t begin(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t syncWord, int8_t power, uint16_t preambleLength, uint8_t gain, float tcxoVoltage) = 0;
    virtual int16_t autoLDRO() = 0;
    virtual int16_t forceLDRO(bool enable) = 0;
    virtual int16_t setCRC(bool enable) = 0;
    virtual int16_t setDataShaping(uint8_t sh) = 0;
};


template <typename T>
class RadioHal : public IRadioHal {
public:
    RadioHal(Module* mod)
    {
        radio = new T(mod);
    }

    int16_t begin(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t syncWord, int8_t power, uint16_t preambleLength, uint8_t gain, float tcxoVoltage);

    int16_t autoLDRO()
    {
        return radio->autoLDRO();
    }

    int16_t forceLDRO(bool enable)
    {
        return radio->forceLDRO(enable);
    }

    int16_t setCRC(bool enable)
    {
        return radio->setCRC(enable);
    }

    int16_t setDataShaping(uint8_t sh)
    {
        return radio->setDataShaping(sh);
    }
    
private:
    T* radio;
};


#endif