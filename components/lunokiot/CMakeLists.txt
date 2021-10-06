set(srcs "./src/LunokIoT.cpp")

#if(LUNOKIOT_ESP32)
    list(APPEND srcs "./src/ESP32/Task.cpp" "./src/ESP32/Device.cpp" "./src/ESP32/Driver.cpp" "./src/ESP32/Service.cpp" "./src/ESP32/Service/NTPService.cpp" "./src/ESP32/Driver/Console.cpp" "./src/ESP32/Driver/I2C.cpp" "./src/ESP32/Driver/NVS.cpp" "./src/ESP32/Driver/WiFi.cpp")
#    target_compile_options(${COMPONENT_LIB} PRIVATE -DLUNOKIOT_ESP32)
#    set(CMAKE_CXX_FLAGS “${CMAKE_CXX_FLAGS} -DLUNOKIOT_ESP32)
#    if(LUNOKIOT_M5STACK_ATOM_LITE)
        list(APPEND srcs "./src/ESP32/Device/M5AtomLite.cpp")
#        add_definitions(-DLUNOKIOT_M5STACK_ATOM_LITE)
#        
#        set(CMAKE_CXX_FLAGS “${CMAKE_CXX_FLAGS} -DLUNOKIOT_M5STACK_ATOM_LITE)
#    endif()    
#endif()

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "./include"
                    REQUIRES "freertos" "esp32 esp_common" "esp_system esp_wifi" "console nvs_flash" "cxx")

#if(LUNOKIOT_ESP32)
#    target_compile_options(${COMPONENT_LIB} PRIVATE -DLUNOKIOT_ESP32)
#endif()
#    target_compile_options(${COMPONENT_LIB} PRIVATE -DLUNOKIOT_M5STACK_ATOM_LITE)