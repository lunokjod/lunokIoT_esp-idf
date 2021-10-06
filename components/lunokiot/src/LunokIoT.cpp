#include <iostream>

//#define LUNOKIOT_M5STACK_ATOM_LITE

#include "LunokIoT.hpp"
#ifdef CONFIG_LUNOKIOT_M5STACK_ATOM_LITE
#include "ESP32/Device/M5AtomLite.hpp"
#endif

void LunokIoT::Begin() {
#ifdef CONFIG_LUNOKIOT_M5STACK_ATOM_LITE
    std::cout << "LOOOL M5StACK!!" << std::endl;
#endif
#ifdef CONFIG_LUNOKIOT_ESP32
    std::cout << "lunokIoT ueAHs" << std::endl;
#endif
    std::cout << "CAGONTOOOO" << std::endl;
#ifdef CONFIG_LUNOKIOT_M5STACK_ATOM_LITE
    M5AtomLiteDevice *a = new M5AtomLiteDevice();
    delete a;
#endif

}
