#include "LunokIoT.hpp"
#include "base/DeviceTemplate.hpp"
#include "ESP32/Device.hpp"

using namespace LunokIoT;

Device::Device(const char* name): DeviceTemplate(name) {
    //printf("%s Device Seup\n", this->name);
}

bool Device::Loop() {
    //printf("%s Device Loop\n", this->name);
    return true;
}
