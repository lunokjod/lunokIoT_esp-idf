#include <LunokIoT.hpp>
#include "ESP32/Device.hpp"
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5AtomLite.hpp"
#include "ESP32/Drivers/Button.hpp"
#include "ESP32/Drivers/SK6812.hpp"

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

#define IR_PIN	gpio_num_t(12)
#define SK6812_PIN	gpio_num_t(27)
#define BUTTON_PIN	gpio_num_t(39)
//@NOTE WARNING https://docs.espressif.com/projects/esp-idf/en/v4.3.1/esp32/api-reference/peripherals/gpio.html?highlight=gpio#_CPPv416gpio_intr_enable10gpio_num_t
/* 
Please do not use the interrupt of GPIO36 and GPIO39 when using ADC or Wi-Fi with 
sleep mode enabled. Please refer to the comments of adc1_get_raw. Please refer 
to section 3.11 of ‘ECO_and_Workarounds_for_Bugs_in_ESP32’ for the description of this issue. 
As a workaround, call adc_power_acquire() in the app. 
This will result in higher power consumption (by ~1mA), but will remove the glitches 
on GPIO36 and GPIO39.
*/


M5AtomLiteDevice::M5AtomLiteDevice(): ESP32Device((const char*)"(-) M5AtomLite") {
    printf("%p %s Setup\n", this, this->name);
    this->_period = 2000;

    printf("%p %s I2C GROVE port setup: $> i2cconfig --sda 26 --scl 32 --freq 10000\n", this, this->name);
    printf("%p %s I2C ATOM-MATE port setup: $> i2cconfig --sda 19 --scl 22 --freq 10000\n", this, this->name);

    this->sk6812 = new SK6812Driver(SK6812_PIN);
    this->button = new ButtonDriver(BUTTON_PIN);

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
