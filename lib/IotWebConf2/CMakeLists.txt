set(COMPONENT_SRCS
        src/IotWebConf.cpp
        src/IotWebConfMultipleWifi.cpp
        src/IotWebConfOptionalGroup.cpp
        src/IotWebConfParameter.cpp
        src/IotWebConfESP32HTTPUpdateServer.cpp
        )

set(COMPONENT_ADD_INCLUDEDIRS
        src/
        )
list(APPEND COMPONENT_REQUIRES "arduino")
register_component()

#ADD_DEFINITIONS(-DESP32)

list(APPEND DEFINITIONS "ESP32")

