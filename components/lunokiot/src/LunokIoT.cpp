#include <iostream>

#include "LunokIoT.hpp"

#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
#include "ESP32/Devices/ESP32.hpp"
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5AtomLite.hpp"
#endif

LunokIoT::Device * LunokIoT::Begin() {
#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
    std::cout << "ESP32 GENERIC DEVICE ENABLED" << std::endl;
    ESP32Device * dev = new ESP32Device();
    return dev;
#endif
#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
    std::cout << "LOOOL M5LITE!!" << std::endl;
    M5AtomLiteDevice *dev = new M5AtomLiteDevice();
    return dev;
#endif
}
