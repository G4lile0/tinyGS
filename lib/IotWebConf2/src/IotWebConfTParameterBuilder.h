/**
 * IotWebConfTParameter.h -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *
 * Copyright (C) 2021 Balazs Kelemen <prampec+arduino@gmail.com>
 *                    rovo89
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef IotWebConfTParameterBuilder_h
#define IotWebConfTParameterBuilder_h

#include <IotWebConfTParameter.h>

namespace iotwebconf
{

template <typename ParamType> class Builder;

template <typename ParamType>
class AbstractBuilder
{
public:
  AbstractBuilder(const char* id) : _id(id) { };
  virtual ParamType build() const
  {
    ParamType instance = std::move(
      ParamType(this->_id, this->_label, this->_defaultValue));
    this->apply(&instance);
    return instance;
  }

  Builder<ParamType>& label(const char* label)
    { this->_label = label; return static_cast<Builder<ParamType>&>(*this); }
  Builder<ParamType>& defaultValue(typename ParamType::DefaultValueType defaultValue)
    { this->_defaultValue = defaultValue; return static_cast<Builder<ParamType>&>(*this); }

protected:
  virtual ParamType* apply(ParamType* instance) const
  {
    return instance;
  }
  const char* _label;
  const char* _id;
  typename ParamType::DefaultValueType _defaultValue;
};

template <typename ParamType>
class Builder : public AbstractBuilder<ParamType>
{
public:
  Builder(const char* id) : AbstractBuilder<ParamType>(id) { };
};

///////////////////////////////////////////////////////////////////////////

template <typename ValueType, typename ParamType>
class PrimitiveBuilder :
  public AbstractBuilder<ParamType>
{
public:
  PrimitiveBuilder<ValueType, ParamType>(const char* id) :
    AbstractBuilder<ParamType>(id) { };
  Builder<ParamType>& min(ValueType min) { this->_minDefined = true; this->_min = min; return static_cast<Builder<ParamType>&>(*this); }
  Builder<ParamType>& max(ValueType max) { this->_maxDefined = true; this->_max = max; return static_cast<Builder<ParamType>&>(*this); }
  Builder<ParamType>& step(ValueType step) { this->_step = step; return static_cast<Builder<ParamType>&>(*this); }
  Builder<ParamType>& placeholder(const char* placeholder) { this->_placeholder = placeholder; return static_cast<Builder<ParamType>&>(*this); }

protected:
  virtual ParamType* apply(
     ParamType* instance) const override
  {
    if (this->_minDefined)
    {
      instance->setMin(this->_min);
    }
    if (this->_maxDefined)
    {
      instance->setMax(this->_max);
    }
    instance->setStep(this->_step);
    instance->setPlaceholder(this->_placeholder);
    return instance;
  }

  bool _minDefined = false;
  bool _maxDefined = false;
  ValueType _min;
  ValueType _max;
  ValueType _step = 0;
  const char* _placeholder = NULL;
};

template <typename ValueType, int base>
class Builder<IntTParameter<ValueType, base>> :
  public PrimitiveBuilder<ValueType, IntTParameter<ValueType, base>>
{
public:
  Builder<IntTParameter<ValueType, base>>(const char* id) :
    PrimitiveBuilder<ValueType, IntTParameter<ValueType, base>>(id) { };
};

template <>
class Builder<FloatTParameter> :
  public PrimitiveBuilder<float, FloatTParameter>
{
public:
  Builder<FloatTParameter>(const char* id) :
    PrimitiveBuilder<float, FloatTParameter>(id) { };
};


template <size_t len>
class Builder<SelectTParameter<len>> :
  public AbstractBuilder<SelectTParameter<len>>
{
public:
  Builder<SelectTParameter<len>>(const char* id) :
    AbstractBuilder<SelectTParameter<len>>(id) { };

  virtual SelectTParameter<len> build() const override
  {
    return SelectTParameter<len>(
      this->_id, this->_label, this->_defaultValue,
      this->_optionValues, this->_optionNames,
      this->_optionCount, this->_nameLength);
  }

  Builder<SelectTParameter<len>>& optionValues(const char* optionValues)
    { this->_optionValues = optionValues; return *this; }
  Builder<SelectTParameter<len>>& optionNames(const char* optionNames)
    { this->_optionNames = optionNames; return *this; }
  Builder<SelectTParameter<len>>& optionCount(size_t optionCount)
    { this->_optionCount = optionCount; return *this; }
  Builder<SelectTParameter<len>>& nameLength(size_t nameLength)
    { this->_nameLength = nameLength; return *this; }

protected:
  virtual SelectTParameter<len>* apply(
     SelectTParameter<len>* instance) const override
  {
    instance->setOptionValues(this->_optionValues);
    instance->setOptionNames(this->_optionNames);
    instance->setOptionCount(this->_optionCount);
    instance->setNameLength(this->_nameLength);
    return instance;
  }

private:
  const char* _optionValues;
  const char* _optionNames;
  size_t _optionCount;
  size_t _nameLength;
};

} // End namespace

#endif
