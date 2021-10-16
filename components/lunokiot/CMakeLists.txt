set(srcs "./src/LunokIoT.cpp")

#esp32 at this time, mandatory (no support for others by now)
list(APPEND srcs "./src/ESP32/Task.cpp" "./src/ESP32/Device.cpp" 
                "./src/ESP32/Driver.cpp" "./src/ESP32/Service.cpp" 
                "./src/ESP32/Services/NTPService.cpp" 
                "./src/ESP32/Drivers/Console.cpp" 
                "./src/ESP32/Drivers/I2C.cpp" 
                "./src/ESP32/Drivers/NVS.cpp" 
                "./src/ESP32/Drivers/WiFi.cpp")
message("---------------------------------------------------------")

if(CONFIG_LUNOKIOT_DEVICE_ESP32)
    list(APPEND srcs "./src/ESP32/Devices/ESP32.cpp")
    message("                     DEVICE TYPE ESP32")
elseif(CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE)
    list(APPEND srcs "./src/ESP32/Devices/ESP32.cpp")
    list(APPEND srcs "./src/ESP32/Devices/M5AtomLite.cpp")
    list(APPEND srcs "./src/ESP32/Drivers/SK6812.cpp")
    list(APPEND srcs "./src/ESP32/Drivers/Button.cpp")
    message("                     DEVICE TYPE M5AtomLite")
elseif(CONFIG_LUNOKIOT_DEVICE_M5STACK_STICK_C_PLUS)
    list(APPEND srcs "./src/ESP32/Devices/ESP32.cpp")
    list(APPEND srcs "./src/ESP32/Devices/M5StickCPlus.cpp")
    list(APPEND srcs "./src/ESP32/Drivers/Button.cpp")
    list(APPEND srcs "./src/ESP32/Drivers/AXP192.cpp")
    message("                     DEVICE TYPE M5StickCPlus")
endif()
message("---------------------------------------------------------")
message("")
message("")
idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "./include"
                    REQUIRES "freertos" "esp32" "esp_common" "driver"
                    "esp_system" "esp_wifi" "console" "nvs_flash")

