#include <iostream>

//#define LUNOKIOT_M5STACK_ATOM_LITE

#include "LunokIoT.hpp"

#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
#include "ESP32/Device/ESP32.hpp"
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
#include "ESP32/Device/M5AtomLite.hpp"
#endif

void LunokIoT::Begin() {
#ifdef CONFIG_LUNOKIOT_ESP32
    std::cout << "ESP32 SUPPORT ENABLED" << std::endl;
#endif
#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
    std::cout << "ESP32 GENERIC DEVICE ENABLED" << std::endl;
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
    std::cout << "LOOOL M5LITE!!" << std::endl;
    M5AtomLiteDevice *a = new M5AtomLiteDevice();
    delete a;
#endif

}
