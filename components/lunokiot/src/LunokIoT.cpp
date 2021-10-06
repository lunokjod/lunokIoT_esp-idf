#include <iostream>

#define LUNOKIOT_M5STACK_ATOM_LITE

#include "LunokIoT.hpp"
#ifdef LUNOKIOT_M5STACK_ATOM_LITE
#include "ESP32/Device/M5AtomLite.hpp"
#endif

void LunokIoT::Begin() {
    std::cout << "CAGONTOOOO" << std::endl;
#ifdef LUNOKIOT_M5STACK_ATOM_LITE
    M5AtomLiteDevice *a = new M5AtomLiteDevice();
    delete a;
#endif

}
