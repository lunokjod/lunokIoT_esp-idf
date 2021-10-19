#include <LunokIoT.hpp>
#include "ESP32/Device.hpp"
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5StickCPlus.hpp"
#include "ESP32/Drivers/Button.hpp"
#include "ESP32/Drivers/AXP192.hpp"
#include "ESP32/Drivers/LED.hpp"

#include <freertos/task.h>
#include <esp_spi_flash.h>
#include <esp_console.h>
#include <argtable3/argtable3.h>
#include <driver/uart.h>
#include <driver/rtc_io.h>
#include <ctype.h>
#include <unistd.h>
#include <esp_sleep.h>
#include <driver/i2c.h>

using namespace LunokIoT;


// https://github.com/lvgl/lv_port_esp32
// https://docs.lvgl.io/latest/en/html/get-started/espressif.html

M5StickCPlusDevice::M5StickCPlusDevice(): ESP32Device((const char*)"(-) M5StickCPlus") {
    printf("%p %s Setup\n", this, name);
    //this->_period = 2000;
    printf("%p %s I2C Hy2.0-4P/GROVE port setup: $> i2cconfig --port 1 --sda 32 --scl 33 --freq 400000\n", this, name);
    printf("%p %s I2C Internal port setup: $> i2cconfig --port 1 --sda 21 --scl 22 --freq 400000\n", this, name);
    printf("%p %s I2C Hat port setup: $> i2cconfig --port 1 --sda 0 --scl 26 --freq 400000\n", this, name);
    this->button0 = new ButtonDriver(BUTTON0_PIN);
    this->button1 = new ButtonDriver(BUTTON1_PIN);

    // AXP192 is connected to internal i2c, must pass the M5StickCPlus paramethers
    this->axp192 = new AXP192Driver(I2C_PORT, I2C_MASTER_FREQ_HZ, I2C1_SDA, I2C1_SCL);
    this->led = new LEDDriver();
    printf("%p %s End Setup\n", this, name);

}
//#define LUNOKIOT_LOG_MARK_TIME_MS 60000

//TickType_t nextRefresh = 0;
//bool M5AtomLiteDevice::Loop() {
    /*
    TickType_t now = (xTaskGetTickCount()/portTICK_RATE_MS);
    if ( nextRefresh < now ) {
        printf("%08ul > --- MARK --- (every %ds)\n", now, LUNOKIOT_LOG_MARK_TIME_MS / 1000);
        nextRefresh = (xTaskGetTickCount()/portTICK_RATE_MS) + LUNOKIOT_LOG_MARK_TIME_MS; // do mark every minute
    }*/
//    return true;
//}
