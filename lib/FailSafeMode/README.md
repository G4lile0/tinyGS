# Fail Safe mode library for ESP8266 and ESP32

![platformio](https://github.com/gmag11/FailSafeMode/workflows/platformio/badge.svg?branch=main)

Sometimes you may have a device that can only be updated via OTA or is very difficult to get physical access to it (because it is in an unreachable place). If you try to send a firmware and it has a bug It may happen that it enters on a boot loop or any other condition that breaks OTA support.

It is really inconvenient and may cause that you end up with an unusable device. I'm sure you know what I mean :)

This library is designed to avoid this on your ESP8266 or ESP32 projects.

It detects boot loops so after a few quick restart cycles it enters in a fail safe mode. Then it starts an AP where you can connect and send a new firmware using OTA.

You only need to add a few lines to your setup and loop functions.

```c++
#include <FailSafe.h>

void setup () {
    FailSafe.checkBoot ();
    if (FailSafe.isActive ()) { // Skip all user setup if fail safe mode is activated
        return;
    }
    
    // Put your setup code here
}

void loop () {
    FailSafe.loop ();
    if (FailSafe.isActive ()) { // Skip all user loop code if Fail Safe mode is active
        return;
    }
    
    // Put your loop code here
}
```



You can start fail safe mode in any moment calling `FailSafe.startFailSafe ();`

