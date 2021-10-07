set(srcs "./src/LunokIoT.cpp")

#esp32
list(APPEND srcs "./src/ESP32/Task.cpp" "./src/ESP32/Device.cpp" "./src/ESP32/Driver.cpp" "./src/ESP32/Service.cpp" "./src/ESP32/Service/NTPService.cpp" "./src/ESP32/Driver/Console.cpp" "./src/ESP32/Driver/I2C.cpp" "./src/ESP32/Driver/NVS.cpp" "./src/ESP32/Driver/WiFi.cpp")

if(CONFIG_LUNOKIOT_DEVICE_ESP32)
    list(APPEND srcs "./src/ESP32/Device/ESP32.cpp")
elseif(CONFIG_LUNOKIOT_M5STACK_ATOM_LITE)
    list(APPEND srcs "./src/ESP32/Device/M5AtomLite.cpp")
endif()

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "./include"
                    REQUIRES "freertos" "esp32 esp_common" "esp_system esp_wifi" "console nvs_flash" "cxx")
