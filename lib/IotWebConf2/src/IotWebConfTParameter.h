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

#ifndef IotWebConfTParameter_h
#define IotWebConfTParameter_h

// TODO: This file is a mess. Help wanted to organize thing!

#include <IotWebConfParameter.h>
#include <Arduino.h>
#include <IPAddress.h>
#include <errno.h>

// At least in PlatformIO, strtoimax/strtoumax are defined, but not implemented.
#if 1
#define strtoimax strtoll
#define strtoumax strtoull
#endif

namespace iotwebconf
{

/**
 * This class is to hide web related properties from the
 *  data manipulation.
 */
class ConfigItemBridge : public ConfigItem
{
public:
  virtual void update(WebRequestWrapper* webRequestWrapper) override
  {
      if (webRequestWrapper->hasArg(this->getId()))
      {
        String newValue = webRequestWrapper->arg(this->getId());
        this->update(newValue);
      }
  }
  void debugTo(Stream* out) override
  {
    out->print("'");
    out->print(this->getId());
    out->print("' with value: '");
    out->print(this->toString());
    out->println("'");
  }

protected:
  ConfigItemBridge(const char* id) : ConfigItem(id) { }
  virtual int getInputLength() { return 0; };
  virtual bool update(String newValue, bool validateOnly = false) = 0;
  virtual String toString() = 0;
};

///////////////////////////////////////////////////////////////////////////

/**
 * DataType is the data related part of the parameter.
 * It does not care about web and visualization, but takes care of the
 *  data validation and storing.
 */
template <typename ValueType, typename _DefaultValueType = ValueType>
class DataType : virtual public ConfigItemBridge
{
public:
  using DefaultValueType = _DefaultValueType;

  DataType(const char* id, DefaultValueType defaultValue) :
    ConfigItemBridge(id),
    _defaultValue(defaultValue)
  {
  }

  /**
   * value() can be used to get the value, but it can also
   * be used set it like this: p.value() = newValue
   */
  ValueType& value() { return this->_value; }
  ValueType& operator*() { return this->_value; }

protected:
  int getStorageSize() override
  {
    return sizeof(ValueType);
  }

  virtual bool update(String newValue, bool validateOnly = false) = 0;
  bool validate(String newValue) { return update(newValue, true); }
  virtual String toString() override { return String(this->_value); }

  ValueType _value;
  const DefaultValueType _defaultValue;
};

///////////////////////////////////////////////////////////////////////////

class StringDataType : public DataType<String>
{
public:
  using DataType<String>::DataType;

protected:
  virtual bool update(String newValue, bool validateOnly) override {
    if (!validateOnly)
    {
      this->_value = newValue;
    }
    return true;
  }
  virtual String toString() override { return this->_value; }
};

///////////////////////////////////////////////////////////////////////////

template <size_t len>
class CharArrayDataType : public DataType<char[len], const char*>
{
public:
using DataType<char[len], const char*>::DataType;
  CharArrayDataType(const char* id, const char* defaultValue) :
    ConfigItemBridge::ConfigItemBridge(id),
    DataType<char[len], const char*>::DataType(id, defaultValue) { };
  virtual void applyDefaultValue() override
  {
    strncpy(this->_value, this->_defaultValue, len);
  }

protected:
  virtual bool update(String newValue, bool validateOnly) override
  {
    if (newValue.length() + 1 > len)
    {
      return false;
    }
    if (!validateOnly)
    {
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
      Serial.print(this->getId());
      Serial.print(": ");
      Serial.println(newValue);
#endif
      strncpy(this->_value, newValue.c_str(), len);
    }
    return true;
  }
  void storeValue(std::function<void(
    SerializationData* serializationData)> doStore) override
  {
    SerializationData serializationData;
    serializationData.length = len;
    serializationData.data = (byte*)this->_value;
    doStore(&serializationData);
  }
  void loadValue(std::function<void(
    SerializationData* serializationData)> doLoad) override
  {
    SerializationData serializationData;
    serializationData.length = len;
    serializationData.data = (byte*)this->_value;
    doLoad(&serializationData);
  }
  virtual int getInputLength() override { return len; };
};

///////////////////////////////////////////////////////////////////////////

/**
 * All non-complex types should be inherited from this base class.
 */
template <typename ValueType>
class PrimitiveDataType : public DataType<ValueType>
{
public:
using DataType<ValueType>::DataType;
  PrimitiveDataType(const char* id, ValueType defaultValue) :
    ConfigItemBridge::ConfigItemBridge(id),
    DataType<ValueType>::DataType(id, defaultValue) { };

  void setMax(ValueType val) { this->_max = val; this->_maxDefined = true; }
  void setMin(ValueType val) { this->_min = val; this->_minDefined = true; }

  virtual void applyDefaultValue() override
  {
    this->_value = this->_defaultValue;
  }

protected:
  virtual bool update(String newValue, bool validateOnly) override
  {
    errno = 0;
    ValueType val = fromString(newValue);
    if ((errno == ERANGE)
      || (this->_minDefined && (val < this->_min))
      || (this->_maxDefined && (val > this->_max)))
    {
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
      Serial.print(this->getId());
      Serial.print(" value not accepted: ");
      Serial.println(val);
#endif
      return false;
    }
    if (!validateOnly)
    {
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
      Serial.print(this->getId());
      Serial.print(": ");
      Serial.println((ValueType)val);
#endif
      this->_value = (ValueType) val;
    }
    return true;
  }
  void storeValue(std::function<void(
    SerializationData* serializationData)> doStore) override
  {
    SerializationData serializationData;
    serializationData.length = this->getStorageSize();
    serializationData.data =
      reinterpret_cast<byte*>(&this->_value);
    doStore(&serializationData);
  }
  void loadValue(std::function<void(
    SerializationData* serializationData)> doLoad) override
  {
    byte buf[this->getStorageSize()];
    SerializationData serializationData;
    serializationData.length = this->getStorageSize();
    serializationData.data = buf;
    doLoad(&serializationData);
    ValueType* valuePointer = reinterpret_cast<ValueType*>(buf);
    this->_value = *valuePointer;
  }
  virtual ValueType fromString(String stringValue) = 0;

  ValueType getMax() { return this->_max; }
  ValueType getMin() { return this->_min; }
  ValueType isMaxDefined() { return this->_maxDefined; }
  ValueType isMinDefined() { return this->_minDefined; }

private:
  ValueType _min;
  ValueType _max;
  bool _minDefined = false;
  bool _maxDefined = false;
};

///////////////////////////////////////////////////////////////////////////

template <typename ValueType, int base = 10>
class SignedIntDataType : public PrimitiveDataType<ValueType>
{
public:
using DataType<ValueType>::DataType;
  SignedIntDataType(const char* id, ValueType defaultValue) :
    ConfigItemBridge::ConfigItemBridge(id),
    PrimitiveDataType<ValueType>::PrimitiveDataType(id, defaultValue) { };

protected:
  virtual ValueType fromString(String stringValue)
  {
    return (ValueType)strtoimax(stringValue.c_str(), NULL, base);
  }
};

template <typename ValueType, int base = 10>
class UnsignedIntDataType : public PrimitiveDataType<ValueType>
{
public:
using DataType<ValueType>::DataType;
  UnsignedIntDataType(const char* id, ValueType defaultValue) :
    ConfigItemBridge::ConfigItemBridge(id),
    PrimitiveDataType<ValueType>::PrimitiveDataType(id, defaultValue) { };

protected:
  virtual ValueType fromString(String stringValue)
  {
    return (ValueType)strtoumax(stringValue.c_str(), NULL, base);
  }
};

class BoolDataType : public PrimitiveDataType<bool>
{
public:
using DataType<bool>::DataType;
  BoolDataType(const char* id, bool defaultValue) :
    ConfigItemBridge::ConfigItemBridge(id),
    PrimitiveDataType<bool>::PrimitiveDataType(id, defaultValue) { };

protected:
  virtual bool fromString(String stringValue)
  {
    return stringValue.c_str()[0] == 1;
  }
};

class FloatDataType : public PrimitiveDataType<float>
{
public:
using DataType<float>::DataType;
  FloatDataType(const char* id, float defaultValue) :
    ConfigItemBridge::ConfigItemBridge(id),
    PrimitiveDataType<float>::PrimitiveDataType(id, defaultValue) { };

protected:
  virtual float fromString(String stringValue)
  {
    return atof(stringValue.c_str());
  }
};

class DoubleDataType : public PrimitiveDataType<double>
{
public:
using DataType<double>::DataType;
  DoubleDataType(const char* id, double defaultValue) :
    ConfigItemBridge::ConfigItemBridge(id),
    PrimitiveDataType<double>::PrimitiveDataType(id, defaultValue) { };

protected:
  virtual double fromString(String stringValue)
  {
    return strtod(stringValue.c_str(), NULL);
  }
};

/////////////////////////////////////////////////////////////////////////

class IpDataType : public DataType<IPAddress>
{
using DataType<IPAddress>::DataType;

protected:
  virtual bool update(String newValue, bool validateOnly) override
  {
    if (validateOnly)
    {
      IPAddress ip;
      return ip.fromString(newValue);
    }
    else
    {
      return this->_value.fromString(newValue);
    }
  }

  virtual String toString() override { return this->_value.toString(); }
};

///////////////////////////////////////////////////////////////////////////

/**
 * Input parameter is the part of the parameter that is responsible
 * for the appearance of the parameter in HTML.
 */
class InputParameter : virtual public ConfigItemBridge
{
public:
  InputParameter(const char* id, const char* label) :
    ConfigItemBridge::ConfigItemBridge(id),
    label(label) { }

  virtual void renderHtml(
    bool dataArrived, WebRequestWrapper* webRequestWrapper) override
  {
    String content = this->renderHtml(
      dataArrived,
      webRequestWrapper->hasArg(this->getId()),
      webRequestWrapper->arg(this->getId()));
    webRequestWrapper->sendContent(content);
  }

  const char* label;

  /**
   * This variable is meant to store a value that is displayed in an empty
   *   (not filled) field.
   */
  const char* placeholder = NULL;
  virtual void setPlaceholder(const char* placeholder) { this->placeholder = placeholder; }

  /**
   * Usually this variable is used when rendering the form input field
   *   so one can customize the rendered outcome of this particular item.
   */
  const char* customHtml = NULL;

  /**
   * Used when rendering the input field. Is is overriden by different
   *   implementations.
   */
  virtual String getCustomHtml()
  {
    return String(customHtml == NULL ? "" : customHtml);
  }

  const char* errorMessage = NULL;

protected:
  void clearErrorMessage() override
  {
    this->errorMessage = NULL;
  }

  virtual String renderHtml(
    bool dataArrived, bool hasValueFromPost, String valueFromPost)
  {
    String pitem = String(this->getHtmlTemplate());

    pitem.replace("{b}", this->label);
    pitem.replace("{t}", this->getInputType());
    pitem.replace("{i}", this->getId());
    pitem.replace(
      "{p}", this->placeholder == NULL ? "" : this->placeholder);
    int length = this->getInputLength();
    if (length > 0)
    {
      char parLength[5];
      snprintf(parLength, 5, "%d", length);
      String maxLength = String("maxlength=") + parLength;
      pitem.replace("{l}", maxLength);
    }
    else
    {
      pitem.replace("{l}", "");
    }
    if (hasValueFromPost)
    {
      // -- Value from previous submit
      pitem.replace("{v}", valueFromPost);
    }
    else
    {
      // -- Value from config
      pitem.replace("{v}", this->toString());
    }
    pitem.replace("{c}", this->getCustomHtml());
    pitem.replace(
        "{s}",
        this->errorMessage == NULL ? "" : "de"); // Div style class.
    pitem.replace(
        "{e}",
        this->errorMessage == NULL ? "" : this->errorMessage);

    return pitem;
  }

  /**
   * One can override this method in case a specific HTML template is required
   * for a parameter.
   */
  virtual String getHtmlTemplate() { return FPSTR(IOTWEBCONF_HTML_FORM_PARAM); };
  virtual const char* getInputType() = 0;
};

template <size_t len>
class TextTParameter : public CharArrayDataType<len>, public InputParameter
{
public:
using CharArrayDataType<len>::CharArrayDataType;
  TextTParameter(const char* id, const char* label, const char* defaultValue) :
    ConfigItemBridge(id),
    CharArrayDataType<len>::CharArrayDataType(id, defaultValue),
    InputParameter::InputParameter(id, label) { }

protected:
  virtual const char* getInputType() override { return "text"; }
};

class CheckboxTParameter : public BoolDataType, public InputParameter
{
public:
  CheckboxTParameter(const char* id, const char* label, const bool defaultValue) :
    ConfigItemBridge(id),
    BoolDataType::BoolDataType(id, defaultValue),
    InputParameter::InputParameter(id, label) { }
  bool isChecked() { return this->value(); }

protected:
  virtual const char* getInputType() override { return "checkbox"; }

  virtual void update(WebRequestWrapper* webRequestWrapper) override
  {
      bool selected = false;
      if (webRequestWrapper->hasArg(this->getId()))
      {
        String valueFromPost = webRequestWrapper->arg(this->getId());
        selected = valueFromPost.equals("selected");
      }
//      this->update(String(selected ? "1" : "0"));
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
      Serial.print(this->getId());
      Serial.print(": ");
      Serial.println(selected ? "selected" : "not selected");
#endif
      this->_value = selected;
  }

  virtual String renderHtml(
    bool dataArrived, bool hasValueFromPost, String valueFromPost) override
  {
    bool checkSelected = false;
    if (dataArrived)
    {
      if (hasValueFromPost && valueFromPost.equals("selected"))
      {
        checkSelected = true;
      }
    }
    else
    {
      if (this->isChecked())
      {
        checkSelected = true;
      }
    }

    if (checkSelected)
    {
      this->customHtml = CheckboxTParameter::_checkedStr;
    }
    else
    {
      this->customHtml = NULL;
    }
    
    return InputParameter::renderHtml(dataArrived, true, String("selected"));
  }
private:
  const char* _checkedStr = "checked='checked'";
};

template <size_t len>
class PasswordTParameter : public CharArrayDataType<len>, public InputParameter
{
public:
using CharArrayDataType<len>::CharArrayDataType;
  PasswordTParameter(const char* id, const char* label, const char* defaultValue) :
    ConfigItemBridge(id),
    CharArrayDataType<len>::CharArrayDataType(id, defaultValue),
    InputParameter::InputParameter(id, label)
  {
    this->customHtml = _customHtmlPwd;
  }

  void debugTo(Stream* out)
  {
    out->print("'");
    out->print(this->getId());
    out->print("' with value: ");
#ifdef IOTWEBCONF_DEBUG_PWD_TO_SERIAL
    out->print("'");
    out->print(this->_value);
    out->println("'");
#else
    out->println(F("<hidden>"));
#endif
  }

  virtual bool update(String newValue, bool validateOnly) override
  {
    if (newValue.length() + 1 > len)
    {
      return false;
    }
    if (validateOnly)
    {
      return true;
    }

#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
    Serial.print(this->getId());
    Serial.print(": ");
#endif
    if (newValue.length() > 0)
    {
      // -- Value was set.
      strncpy(this->_value, newValue.c_str(), len);
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
# ifdef IOTWEBCONF_DEBUG_PWD_TO_SERIAL
      Serial.println(this->_value);
# else
      Serial.println("<updated>");
# endif
#endif
    }
    else
    {
#ifdef IOTWEBCONF_DEBUG_TO_SERIAL
      Serial.println("<was not changed>");
#endif
    }
    return true;
  }

protected:
  virtual const char* getInputType() override { return "password"; }
  virtual String renderHtml(
    bool dataArrived, bool hasValueFromPost, String valueFromPost) override
  {
    return InputParameter::renderHtml(dataArrived, true, String(""));
  }
private:
  const char* _customHtmlPwd = "ondblclick=\"pw(this.id)\"";
};

/**
 * All non-complex type input parameters should be inherited from this
 *  base class.
 */
template <typename ValueType>
class PrimitiveInputParameter :
  public InputParameter
{
public:
  PrimitiveInputParameter(const char* id, const char* label) :
    ConfigItemBridge::ConfigItemBridge(id),
    InputParameter::InputParameter(id, label) { }

  virtual String getCustomHtml() override
  {
    String modifiers = String(this->customHtml);

    if (this->isMinDefined())
    {
      modifiers += " min='" ;
      modifiers += this->getMin();
      modifiers += "'";
    }
    if (this->isMaxDefined())
    {
      modifiers += " max='";
      modifiers += this->getMax();
      modifiers += "'";
    }
    if (this->step != 0)
    {
      modifiers += " step='";
      modifiers += this->step;
      modifiers += "'";
    }

    return modifiers;
  }

  ValueType step = 0;
  void setStep(ValueType step) { this->step = step; }
  virtual ValueType getMin() = 0;
  virtual ValueType getMax() = 0;
  virtual bool isMinDefined() = 0;
  virtual bool isMaxDefined() = 0;
};

template <typename ValueType, int base = 10>
class IntTParameter :
  public virtual SignedIntDataType<ValueType, base>,
  public PrimitiveInputParameter<ValueType>
{
public:
  IntTParameter(const char* id, const char* label, ValueType defaultValue) :
    ConfigItemBridge(id),
    SignedIntDataType<ValueType, base>::SignedIntDataType(id, defaultValue),
    PrimitiveInputParameter<ValueType>::PrimitiveInputParameter(id, label) { }

  // TODO: somehow organize these methods into common parent.
  virtual ValueType getMin() override
  {
    return PrimitiveDataType<ValueType>::getMin();
  }
  virtual ValueType getMax() override
  {
    return PrimitiveDataType<ValueType>::getMax();
  }

  virtual bool isMinDefined() override
  {
    return PrimitiveDataType<ValueType>::isMinDefined();
  }
  virtual bool isMaxDefined() override
  {
    return PrimitiveDataType<ValueType>::isMaxDefined();
  }

protected:
  virtual const char* getInputType() override { return "number"; }
};

template <typename ValueType, int base = 10>
class UIntTParameter :
  public virtual UnsignedIntDataType<ValueType, base>,
  public PrimitiveInputParameter<ValueType>
{
public:
  UIntTParameter(const char* id, const char* label, ValueType defaultValue) :
    ConfigItemBridge(id),
    SignedIntDataType<ValueType, base>::SignedIntDataType(id, defaultValue),
    PrimitiveInputParameter<ValueType>::PrimitiveInputParameter(id, label) { }

  // TODO: somehow organize these methods into common parent.
  virtual ValueType getMin() override
  {
    return PrimitiveDataType<ValueType>::getMin();
  }
  virtual ValueType getMax() override
  {
    return PrimitiveDataType<ValueType>::getMax();
  }

  virtual bool isMinDefined() override
  {
    return PrimitiveDataType<ValueType>::isMinDefined();
  }
  virtual bool isMaxDefined() override
  {
    return PrimitiveDataType<ValueType>::isMaxDefined();
  }

protected:
  virtual const char* getInputType() override { return "number"; }
};

class FloatTParameter :
  public FloatDataType,
  public PrimitiveInputParameter<float>
{
public:
  FloatTParameter(const char* id, const char* label, float defaultValue) :
    ConfigItemBridge(id),
    FloatDataType::FloatDataType(id, defaultValue),
    PrimitiveInputParameter<float>::PrimitiveInputParameter(id, label) { }

  virtual float getMin() override
  {
    return PrimitiveDataType<float>::getMin();
  }
  virtual float getMax() override
  {
    return PrimitiveDataType<float>::getMax();
  }

  virtual bool isMinDefined() override
  {
    return PrimitiveDataType<float>::isMinDefined();
  }
  virtual bool isMaxDefined() override
  {
    return PrimitiveDataType<float>::isMaxDefined();
  }

protected:
  virtual const char* getInputType() override { return "number"; }
};

/**
 * Options parameter is a structure, that handles multiple values when redering
 * the HTML representation.
 */
template <size_t len>
class OptionsTParameter : public TextTParameter<len>
{
public:
  /**
   * @optionValues - List of values to choose from with, where each value
   *   can have a maximal size of 'length'. Contains 'optionCount' items.
   * @optionNames - List of names to render for the values, where each
   *   name can have a maximal size of 'nameLength'. Contains 'optionCount'
   *   items.
   * @optionCount - Size of both 'optionValues' and 'optionNames' lists.
   * @nameLength - Size of any item in optionNames list.
   * (See TextParameter for arguments!)
   */
  OptionsTParameter(
    const char* id, const char* label, const char* defaultValue,
    const char* optionValues, const char* optionNames,
    size_t optionCount, size_t nameLength) :
    ConfigItemBridge(id),
    TextTParameter<len>(id, label, defaultValue)
  {
    this->_optionValues = optionValues;
    this->_optionNames = optionNames;
    this->_optionCount = optionCount;
    this->_nameLength = nameLength;
  }

  // TODO: make these protected
  void setOptionValues(const char* optionValues) { this->_optionValues = optionValues; }
  void setOptionNames(const char* optionNames) { this->_optionNames = optionNames; }
  void setOptionCount(size_t optionCount) { this->_optionCount = optionCount; }
  void setNameLength(size_t nameLength) { this->_nameLength = nameLength; }
protected:
  OptionsTParameter(
    const char* id, const char* label, const char* defaultValue) :
    ConfigItemBridge(id),
    TextTParameter<len>(id, label, defaultValue)
  {
  }

  const char* _optionValues;
  const char* _optionNames;
  size_t _optionCount;
  size_t _nameLength;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Select parameter is an option parameter, that rendered as HTML SELECT.
 * Basically it is a dropdown combobox.
 */
template <size_t len>
class SelectTParameter : public OptionsTParameter<len>
{
public:
  /**
   * Create a select parameter for the config portal.
   *
   * (See OptionsParameter for arguments!)
   */
  SelectTParameter(
    const char* id, const char* label, const char* defaultValue,
    const char* optionValues, const char* optionNames,
    size_t optionCount, size_t nameLength) :
    ConfigItemBridge(id),
    OptionsTParameter<len>(
      id, label, defaultValue, optionValues, optionNames, optionCount, nameLength)
    { }
  // TODO: make this protected
  SelectTParameter(
    const char* id, const char* label, const char* defaultValue) :
    ConfigItemBridge(id),
    OptionsTParameter<len>(id, label, defaultValue) { }

protected:
  // Overrides
  virtual String renderHtml(
    bool dataArrived, bool hasValueFromPost, String valueFromPost) override
  {
    String options = "";

    for (size_t i=0; i<this->_optionCount; i++)
    {
      const char *optionValue = (this->_optionValues + (i*len) );
      const char *optionName = (this->_optionNames + (i*this->_nameLength) );
      String oitem = FPSTR(IOTWEBCONF_HTML_FORM_OPTION);
      oitem.replace("{v}", optionValue);
//    if (sizeof(this->_optionNames) > i)
      {
        oitem.replace("{n}", optionName);
      }
//    else
//    {
//      oitem.replace("{n}", "?");
//    }
      if ((hasValueFromPost && (valueFromPost == optionValue)) ||
        (strncmp(this->value(), optionValue, len) == 0))
      {
        // -- Value from previous submit
        oitem.replace("{s}", " selected");
      }
      else
      {
        // -- Value from config
        oitem.replace("{s}", "");
      }

      options += oitem;
    }

    String pitem = FPSTR(IOTWEBCONF_HTML_FORM_SELECT_PARAM);

    pitem.replace("{b}", this->label);
    pitem.replace("{i}", this->getId());
    pitem.replace(
        "{c}", this->customHtml == NULL ? "" : this->customHtml);
    pitem.replace(
        "{s}",
        this->errorMessage == NULL ? "" : "de"); // Div style class.
    pitem.replace(
        "{e}",
        this->errorMessage == NULL ? "" : this->errorMessage);
    pitem.replace("{o}", options);

    return pitem;
  }

private:
};


} // end namespace

#include <IotWebConfTParameterBuilder.h>

#endif
