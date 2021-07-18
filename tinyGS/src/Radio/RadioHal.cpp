#include "RadioHal.hpp"

template<>
int16_t RadioHal<SX1278>::begin(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t syncWord, int8_t power, uint16_t preambleLength, uint8_t gain, float tcxoVoltage)
{
    return radio->begin(freq, bw, sf, cr, syncWord, power, preambleLength, gain);
}

template<>
int16_t RadioHal<SX1268>::begin(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t syncWord, int8_t power, uint16_t preambleLength, uint8_t gain, float tcxoVoltage)
{
    return radio->begin(freq, bw, sf, cr, syncWord, power, preambleLength, tcxoVoltage);
}

template<>
int16_t RadioHal<SX1278>::beginFSK(float freq, float br, float freqDev, float rxBw, int8_t power, uint16_t preambleLength, bool enableOOK, float tcxoVoltage, bool useRegulatorLDO)
{
    return radio->beginFSK(freq, br, freqDev, rxBw, power, preambleLength, enableOOK);
}

template<>
int16_t RadioHal<SX1268>::beginFSK(float freq, float br, float freqDev, float rxBw, int8_t power, uint16_t preambleLength, bool enableOOK, float tcxoVoltage, bool useRegulatorLDO)
{
    return radio->beginFSK(freq, br, freqDev, rxBw, power, preambleLength, tcxoVoltage, useRegulatorLDO);
}

template<>
float RadioHal<SX1280>::getRSSI(bool skipReceive)
{
    return radio->getRSSI();
}

template<>
float RadioHal<SX1268>::getRSSI(bool skipReceive)
{
    return radio->getRSSI();
}

template<>
float RadioHal<SX1278>::getRSSI(bool skipReceive)
{
    return radio->getRSSI(skipReceive);
}

template<>
float RadioHal<SX1268>::getFrequencyError(bool autoCorrect)
{
    return 0;
}

template<>
float RadioHal<SX1278>::getFrequencyError(bool autoCorrect)
{
    return radio->getFrequencyError(autoCorrect);
}

template<>
void RadioHal<SX1268>::setDio0Action(void (*func)(void))
{
    radio->setDio1Action(func);
}

template<>
void RadioHal<SX1278>::setDio0Action(void (*func)(void))
{
    radio->setDio0Action(func);
}

template<>
int16_t RadioHal<SX1278>::startReceive(uint8_t len, uint8_t mode)
{
    return radio->startReceive(len, mode);
}

template<>
int16_t RadioHal<SX1268>::startReceive(uint8_t len, uint8_t mode)
{
    return radio->startReceive(len);
}