#include "LunokIoT.hpp"
#include "ESP32/Service.hpp"
#include "ESP32/Service/NTPService.hpp"

using namespace LunokIoT;

NTPService::NTPService() : Service((const char *)"(-) NTP Service") {
    //Reconfigure task to = (unsigned long)CONFIG_LWIP_SNTP_UPDATE_DELAY
    //printf("%s Driver Seup\n", this->name);
}

bool NTPService::Loop() {
    //printf("%s Driver Loop\n", this->name);
    return true;
}
