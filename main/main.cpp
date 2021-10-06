#include <iostream>
#define LUNOKIOT_ESP32
#define LUNOKIOT_M5STACK_ATOM_LITE
#include <LunokIoT.hpp>

extern "C" void app_main(void)
{
    std::cout << "app_main starting" << std::endl;
    LunokIoT::Begin();
    std::cout << "app_main done" << std::endl;

}
