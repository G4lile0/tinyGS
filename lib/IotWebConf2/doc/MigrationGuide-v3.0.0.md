# Migration guide to v3.0.0

In v3.0.0 some changes were introduced, that are not backward
compatible with v2.x.x versions.
This guide contains all modifications that should be done in existing
codes, to reflect the changes.

For better understanding some code examples are also shown here, but
I would recommend comparing git changes in the examples. 

## Changes introduced in v3.0.0

  - [Namespaces](#namespaces)
  - [Parameter classes](#parameter-classes)
  - [Parameter grouping](#grouping-parameters)
  - [Default value handling](#default-value-handling)
  - [Hidden parameters](#hidden-parameters)
  - [UpdateServer changes](#updateserver-changes)
  - [configSave](#configsave)
  - [formValidator](#formvalidator)
  
## Namespaces

With v3.0.0, IotWebConf library started to use namespaces. Namespace
is a C++ technique, where to goal is to avoid name collision over
different libraries.

The namespace for IotWebConf become ```iotwebconf::```. From now on
you should use this prefix for each type defined by the library
except for the IotWebConf class itself.

### Migration steps: easy way
For easy migration IotWebConf has provided a header file prepared
with predefined aliases to hide namespaces.
Include helper header file as follows.

Code before:
```C++
#include <IotWebConf.h>
```

Code after:
```C++
#include <IotWebConf.h>
#include <IotWebConfUsing.h>
```

### Migration steps: proper way
Use namespace prefixes as follows.

Code before:
```C++
class CustomHtmlFormatProvider :
  public IotWebConfHtmlFormatProvider
{
protected:
...
```

Code after:
```C++
class CustomHtmlFormatProvider :
  public iotwebconf::HtmlFormatProvider
{
protected:
...
```

## Parameter classes

Previously there was just the ```IotWebConfParameter``` and the
parameter was provided as an argument. Now it turned out, that 
it is a better idea to use specific classes for each individual
 types. So from now on you must specify the
 type of
the parameter by creating that very type e.g. using 
```IotWebConfTextParameter```.
 
For compatibility reasons the signature is the same before, except
the type string should not be provided anymore.

New parameter types are also introduced (e.g.
 ```IotWebConfSelectParameter```),
and it is very likely that with newer versions, more and more types will
 arrive.
Creating your custom parameter is now become much more easy as well.

### Migrations steps
Replace IotWebConfParameter types with specific parameter type.

Code before:
```C++
IotWebConfParameter mqttServerParam =
  IotWebConfParameter("MQTT server", "mqttServer", mqttServerValue , STRING_LEN);
IotWebConfParameter mqttUserPasswordParam =
  IotWebConfParameter("MQTT password", "mqttPass", mqttUserPasswordValue , STRING_LEN, "password");
```

Code after:
```C++
IotWebConfTextParameter mqttServerParam =
  IotWebConfTextParameter("MQTT server", "mqttServer", mqttServerValue , STRING_LEN);
IotWebConfPasswordParameter mqttUserPasswordParam =
  IotWebConfPasswordParameter("MQTT password", "mqttPass ", mqttUserPasswordValue, STRING_LEN);
```

Note, that ```IotWebConfTextParameter``` and
```IotWebConfPasswordParameter``` words are just aliases and eventually you
should use ```iotwebconf::TextParameter```,
```iotwebconf::PasswordParameter```, etc.


## Grouping parameters

With v3.0.0 "separator" disappears. Separators were used to create
field sets in the rendered HTML. Now you must directly define connected
items by adding them to specific parameter groups.
(It is also possible to add a group within a group.)
 
You need to add prepared groups to IotWebConf instead of individual
parameters. (However there is a specific group created by IotWebConf for
storing system parameters, you can also add your
properties into the system group.)

Code before:
```C++
IotWebConfSeparator separator1 =
  IotWebConfSeparator();
IotWebConfParameter intParam =
  IotWebConfParameter("Int param", "intParam", intParamValue, NUMBER_LEN, "number",
    "1..100", NULL, "min='1' max='100' step='1'");

...

void setup() 
{
...
  iotWebConf.addParameter(&separator1);
  iotWebConf.addParameter(&intParam);
...
```

Code after:
```C++
IotWebConfParameterGroup group1 =
  IotWebConfParameterGroup("group1", "");
IotWebConfNumberParameter intParam =
  IotWebConfNumberParameter("Int param", "intParam", intParamValue, NUMBER_LEN,
    "20", "1..100", "min='1' max='100' step='1'");

...

void setup() 
{
...
  group1.addItem(&intParam);
...
  iotWebConf.addParameterGroup(&group1);
...
```

Also note, that ```IotWebConfParameterGroup``` and
```IotWebConfNumberParameter``` words are just aliases and eventually you
should use ```iotwebconf::ParameterGroup```,
```iotwebconf::NumberParameter```, etc.

## Default value handling

For the Parameters you could always specify "defaultValue". In v2.x
.x this value was intended to be appeared in the config portal, if no
values are specified. On the other hand in v3.0.0 defaultValue is
automatically assigned to the parameter, when this is the first
time configuration is loading.

This means you do not have to set these values manually.

In the example below, the body of the ```if``` is done by IotWebConf
automatically.
```
  // -- Initializing the configuration.
  bool validConfig = iotWebConf.init();
  if (!validConfig)
  {
    // DO NOT DO THIS! Use default values instead.
    strncpy(mqttServerValue, "192.168.1.10", STRING_LEN);
  }
```

## Hidden parameters

IotWebConf can save and load parameters, that are not populated to the
web interface. To mark an item as hidden, you should have set the last
parameter of the constructor to visible=false.

From v3.0.0, you will need to add hidden items to a specific group managed
by IotWebConf.
```
iotWebConf.addHiddenParameter(&myHiddenParameter);
```

## UpdateServer changes

In prior versions, IotWebConf switched on HTTP Update server. With
version 3.0.0, IotWebConf dropped the dependency to UpdateServer. The
switching will still be triggered, but implementations should be provided
to do the actual switching.

A quite complicated code needs to introduced because of this, and you
need to manually include UpdateServer to your code. See example:
 ```IotWebConf04UpdateServer``` for details!

```
  iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });
```

Note, that ESP32 still doesn't provide Update Server solution out of the
box. IotWebConf still provides an implementation for that, but it is now
completely independent from the core codes.

## configSave
Method configSave is renamed to saveConfig.

## formValidator
The formValidator() methods from now on will have a WebRequestWrapper*
parameter.