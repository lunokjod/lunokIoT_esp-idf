#include <LunokIoT.hpp>
#include "ESP32/Device.hpp"
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5AtomLite.hpp"
#include "ESP32/Drivers/NVS.hpp"
#include "ESP32/Drivers/Console.hpp"
#include "ESP32/Drivers/I2C.hpp"
#include "ESP32/Services/NTPService.hpp"

#include <freertos/task.h>
#include <esp_spi_flash.h>
#include <esp_console.h>
#include <argtable3/argtable3.h>
#include <driver/uart.h>
#include <driver/rtc_io.h>
#include <ctype.h>
#include <unistd.h>
#include <esp_sleep.h>

using namespace LunokIoT;

M5AtomLiteDevice::M5AtomLiteDevice(): ESP32Device((const char*)"(-) M5AtomLite") {
    printf("%p %s Setup\n", this, this->name);
    this->_period = 2000;


    printf("%p %s I2C default port setup: $> i2cconfig --sda 25 --scl 21 --freq 10000\n", this, this->name);
    printf("%p %s End Setup\n", this, this->name);
}
#define LUNOKIOT_LOG_MARK_TIME_MS 60000

//TickType_t nextRefresh = 0;z
//bool M5AtomLiteDevice::Loop() {
    /*
    TickType_t now = (xTaskGetTickCount()/portTICK_RATE_MS);
    if ( nextRefresh < now ) {
        printf("%08ul > --- MARK --- (every %ds)\n", now, LUNOKIOT_LOG_MARK_TIME_MS / 1000);
        nextRefresh = (xTaskGetTickCount()/portTICK_RATE_MS) + LUNOKIOT_LOG_MARK_TIME_MS; // do mark every minute
    }*/
//    return true;
//}
