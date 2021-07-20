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
int16_t RadioHal<SX1280>::begin(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t syncWord, int8_t power, uint16_t preambleLength, uint8_t gain, float tcxoVoltage)
{
    return radio->begin(freq, bw, sf, cr, syncWord, power, preambleLength);
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
int16_t RadioHal<SX1280>::beginFSK(float freq, float br, float freqDev, float rxBw, int8_t power, uint16_t preambleLength, bool enableOOK, float tcxoVoltage, bool useRegulatorLDO)
{
    return radio->beginGFSK(freq, br, freqDev, power, preambleLength);
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
float RadioHal<SX1280>::getFrequencyError(bool autoCorrect)
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
void RadioHal<SX1280>::setDio0Action(void (*func)(void))
{
    radio->setDio1Action(func);
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

template<>
int16_t RadioHal<SX1280>::startReceive(uint8_t len, uint8_t mode)
{
    return radio->startReceive(len);
}

template<>
int16_t RadioHal<SX1280>::autoLDRO()
{
    return 0;
}

template<>
int16_t RadioHal<SX1278>::autoLDRO()
{
    return radio->autoLDRO();
}

template<>
int16_t RadioHal<SX1268>::autoLDRO()
{
    return radio->autoLDRO();
}

template<>
int16_t RadioHal<SX1278>::forceLDRO(bool enable)
{
    return radio->forceLDRO(enable);
}

template<>
int16_t RadioHal<SX1268>::forceLDRO(bool enable)
{
    return radio->forceLDRO(enable);
}

template<>
int16_t RadioHal<SX1280>::forceLDRO(bool enable)
{
    return 0;
}

template<>
int16_t RadioHal<SX1278>::fixedPacketLengthMode(uint8_t len)
{
    return radio->fixedPacketLengthMode(len);
}

template<>
int16_t RadioHal<SX1268>::fixedPacketLengthMode(uint8_t len)
{
    return radio->fixedPacketLengthMode(len);
}

template<>
int16_t RadioHal<SX1280>::fixedPacketLengthMode(uint8_t len)
{
    return 0;
}