#include "LunokIoT.hpp"
#include "base/ServiceTemplate.hpp"
#include "ESP32/Service.hpp"

using namespace LunokIoT;

Service::Service(const char* name): ServiceTemplate(name) {
    //printf("%s Driver Seup\n", this->name);
}

bool Service::Loop() {
    //printf("%s Driver Loop\n", this->name);
    return true;
}
