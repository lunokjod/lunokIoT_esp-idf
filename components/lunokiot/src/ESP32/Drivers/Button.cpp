#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/Button.hpp"

using namespace LunokIoT;


ButtonDriver::ButtonDriver(gpio_num_t gpio): Driver((const char*)"(-) Button", (unsigned long)-1), gpio(gpio) {
    printf("%p %s Setup\n", this, this->name);
}
bool ButtonDriver::Loop() {
    return true;
}
