#include <iostream>
#include <LunokIoT.hpp>

extern "C" void app_main(void)
{
    std::cout << "app_main starting" << std::endl;
    LunokIoT::Begin();
    std::cout << "app_main done" << std::endl;

}
