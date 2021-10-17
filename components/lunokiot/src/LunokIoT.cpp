#include <iostream>

#include "LunokIoT.hpp"

#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
#include "ESP32/Devices/ESP32.hpp"
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5AtomLite.hpp"
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_STICK_C_PLUS
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5StickCPlus.hpp"
#endif

void LunokIoT::Begin() {
    std::cout << "lunokIoT device: "; 
#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
    std::cout << "ESP32 generic" << std::endl;
    //ESP32Device * dev = 
    new ESP32Device();
    return;
#endif
#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
    std::cout << "M5Atom Lite" << std::endl;
    //M5AtomLiteDevice *dev = 
    new M5AtomLiteDevice();
    return;
#endif
#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_STICK_C_PLUS
    std::cout << "M5StickC Plus" << std::endl;
    //M5StickCPlusDevice *dev = 
    new M5StickCPlusDevice();
    return;
#endif
}
