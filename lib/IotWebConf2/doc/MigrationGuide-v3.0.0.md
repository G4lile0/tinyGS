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

There are more ways to update your code. Let's see some variations!

### Migration steps: easy way
For easy migration IotWebConf has provided a header file prepared
with predefined aliases to hide namespaces, so you can still use
the legacy types.

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
Use namespace prefixes before every type name.

Code before:
```C++
IotWebConfParameter mqttServerParam =
  IotWebConfParameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
```

Code after:
```C++
iotwebconf::Parameter mqttServerParam =
  iotwebconf::Parameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
```

### Migration steps: optimist way
Define namespaces at the beginning of the code and use simple type name.
Everywhere later on. This works only until name-collision with other
library occur.

Code after:
```C++
using namespace iotwebconf;
...
Parameter mqttServerParam =
  Parameter("MQTT server", "mqttServer", mqttServerValue, STRING_LEN);
```

## Parameter classes

Previously there was just the ```IotWebConfParameter``` and the
actual type was provided as an argument of this one-and-only type.
Now it turned out, that it is a better idea to use specific classes
for each individual types. So from now on you must specify the type
of the parameter by creating that very type e.g. using 
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

_Note, that with version 3.0.0 a new typed parameter approach is introduced,
you might want to immediately migrate to this parameter types, but
typed-parameters are still in testing phase and might be a subject of
change._ 

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
values are specified. Now with v3.0.0, defaultValue has a different
meaning. Now it is
automatically assigned to the parameter, when this is the **first
time** configuration is loading.

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

In prior versions, IotWebConf activated HTTP Update server automatically.
With version 3.0.0, IotWebConf dropped the dependency to UpdateServer. The
activation will still be triggered, but the actual switching action
should be provided externally (at your code).

A quite complicated code needs to introduced because of this change, and
you need to manually include UpdateServer to your code. See example:
 ```IotWebConf04UpdateServer``` for details!

Changed lines:

```
// Include Update server
#ifdef ESP8266
# include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
# include <IotWebConfESP32HTTPUpdateServer.h>
#endif

// Create Update Server
#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
HTTPUpdateServer httpUpdater;
#endif

  // In setup register callbacks performing Update Server hooks. 
  iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });
```

Note, that ESP32 still doesn't provide Update Server solution out of the
box. IotWebConf still provides an implementation for that, but it is now
completely independent of the core codes.

## configSave
Method configSave is renamed to saveConfig.

## formValidator
The formValidator() methods from now on will have a
```webRequestWrapper``` parameter.

```
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);
```