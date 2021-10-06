#include "LunokIoT.hpp"
#include "base/DriverTemplate.hpp"
#include "ESP32/Driver.hpp"

using namespace LunokIoT;

Driver::Driver(const char* name, unsigned long period): DriverTemplate(name, period) {
    //printf("%s Driver Seup\n", this->name);
}

bool Driver::Loop() {
    //printf("%s Driver Loop\n", this->name);
    return true;
}
