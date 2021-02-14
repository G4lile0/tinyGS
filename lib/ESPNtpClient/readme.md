[![PlatformIO](https://github.com/gmag11/ESPNtpClient/workflows/PlatformIO/badge.svg)](https://github.com/gmag11/ESPNtpClient/actions/)

# ESPNtpClient

There are many NTP client libraries around. All them have different implementations for time tracking. Indeed I developed one of them. It is called [NTPClientLib](https://github.com/gmag11/NtpClient) and was intended to be as easy to use as possible while offering a wide range of features, including multiplatform support (ESP8266, ESP32, Arduino MKR1000, Arduino UNO with Ethernet shield). Since that I moved all my projects to ESP8266 and ESP32.

Some time ago ESP32 and ESP8266 SDKs include NTP and time tracking internal functions, including Posix compliant Time.h implementation, what gives less sense to use external libraries.

But all those libraries including Espressif SDK synchronize clock with plus minus 1 second accuracy, what is enough for most projects. But some others that require tight synchronization require additional sources of time reference, although NTP protocol is suitable to have 1 millisecond accuracy on these little boards.

This limitation comes from the way they calculate time offset. All them (at least the ones that I know) only use one of three timestamps that can be extracted from NTP packet, so they do not have round trip delay into account.

This library implementation uses time offset calculation as stated in [RFC5905 NTP standard](https://tools.ietf.org/html/rfc5905). It is not a complete implementation of NTP protocol in other aspects, although this allows getting precision very close to 1 millisecond.

Many of the methods used in my old NTPClientLib library are the same in this one. I've added some methods to get `timeval` times, that include microseconds information.

ESPNtpClient do not have any external dependency.

----------------

**Important**: This library task is only related to time synchronization and, as NTP protocol does, works using UTC time internally. All time management including local time conversion, time zones, daylight savings, etc. is done by Espressif sdk time subsystem that is based on GNU time C library. https://kirste.userpage.fu-berlin.de/chemnet/use/info/libc/libc_17.html

----------

(Attribution) Clock logo taken from https://www.visualpharm.com/free-icons/clock-595b40b75ba036ed117d92ff

## Description

This is a NTP library to be able to get time from NTP server with my connected microcontrollers. 

Using the library is fairly easy. A NTP singleton object is created inside library. You may use default values by using `NTP.begin()` without any parameter. After that, synchronization is done regularly without user intervention. Some parameters can be adjusted: server, sync frequency, time zone.

You don't need anything more. Time update is managed inside library so, after `NTP.begin()` no more calls to library are needed.

Update frequency is higher (every 15 seconds as default) until 1st successful sync is achieved. Since then, your own (or default 1800 seconds = half hour) adjusted period applies. There is a way to adjust both short and long sync period if needed.

Mostly, this is compatible with older [NTPClientLib](https://github.com/gmag11/NtpClient) library.

This library includes an uptime log too. It counts number of seconds since sketch is started.

Every time that local time is adjusted a `ntpEvent` is thrown. You can attach a function to it using `NTP.onNTPSyncEvent()`. Called function format must be like `void eventHandler(NTPSyncEvent_t event)`.

Library does WiFi connection tracking by itself so you can call begin after or before WiFi is connected and it takes care of WiFi reconnections. Meanwhile, if 'NTP.begin()' is called when WiFi is already connected, it takes far less to get syncronization. It takes up to 30 seconds if library is called before WiFi connection is completed, but it will only take less than 5 seconds if Wifi was connected prior to `NTP.begin()` call

There are two examples, one simple and minimum one to show the very basic implementation. Second one shows advanced use with event and WiFi state management.



