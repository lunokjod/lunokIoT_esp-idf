#include "LunokIoT.hpp"
#include "ESP32/Device.hpp"
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/LilygoTWatch2020v3.hpp"
#include "ESP32/Drivers/Button.hpp"
#include "ESP32/Drivers/AXP202.hpp"

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
// https://github.com/tuupola/axp202/blob/master/axp202.h
#define IR_PIN	gpio_num_t(13)
//@NOTE button is AXP202 PEK

//@NOTE WARNING https://docs.espressif.com/projects/esp-idf/en/v4.3.1/esp32/api-reference/peripherals/gpio.html?highlight=gpio#_CPPv416gpio_intr_enable10gpio_num_t
/* 
Please do not use the interrupt of GPIO36 and GPIO39 when using ADC or Wi-Fi with 
sleep mode enabled. Please refer to the comments of adc1_get_raw. Please refer 
to section 3.11 of ‘ECO_and_Workarounds_for_Bugs_in_ESP32’ for the description of this issue. 
As a workaround, call adc_power_acquire() in the app. 
This will result in higher power consumption (by ~1mA), but will remove the glitches 
on GPIO36 and GPIO39.
*/

// https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/docs/watch_2020_v3.md

LilygoTWatch2020v3Device::LilygoTWatch2020v3Device(): ESP32Device((const char*)"(-) LilygoTWatch2020v3Device") {
    debug_printf("Setup");
    this->_period = 2000;
    debug_printf("I2C default port setup: $> i2cconfig --port 1 --sda 21 --scl 22 --freq 400000");
    debug_printf("I2C touch port setup: $> i2cconfig --port 1 --sda 23 --scl 32 --freq 400000");

    // AXP202
    this->axp202 = new AXP202Driver(this->i2c, I2C_PORT, I2C_MASTER_FREQ_HZ, I2C0_SDA, I2C0_SCL);


    debug_printf("End Setup");
#ifdef CONFIG_LUNOKIOT_DEVICE_LILYGO_TWATCH_2020_V3
    this->console->EnableConsole();
#endif // CONFIG_LUNOKIOT_DEVICE_LILYGO_TWATCH_2020_V3

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
