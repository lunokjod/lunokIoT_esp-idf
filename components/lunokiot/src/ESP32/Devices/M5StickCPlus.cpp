#include <LunokIoT.hpp>
#include "ESP32/Device.hpp"
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5StickCPlus.hpp"
#include "ESP32/Drivers/Button.hpp"
#include "ESP32/Drivers/AXP192.hpp"

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

#define BUZZER_PIN	gpio_num_t(2)
#define IR_PIN	gpio_num_t(10)
#define LED_PIN	gpio_num_t(27)
#define BUTTON0_PIN	gpio_num_t(37)
#define BUTTON1_PIN	gpio_num_t(39)
// https://github.com/m5stack/M5StickC/blob/master/examples/Advanced/AXP192/BtnIrq/BtnIrq.ino

#define MIC_CLK_PIN	gpio_num_t(0)
#define MIC_DATA_PIN	gpio_num_t(34)

#define TFT_MOSI gpio_num_t(15)
#define TFT_CLK gpio_num_t(13)
#define TFT_DC gpio_num_t(23)
#define TFT_RST gpio_num_t(18)
#define TFT_CS gpio_num_t(5)

/*
    this->DetectI2CHardware("Local", 21,22);
    this->DetectI2CHardware("Hy2.0-4P", 32,33);
    this->DetectI2CHardware("Hat", 0,26);
*/
// Hy2.0-4P/Grove
#define I2C0_SCL gpio_num_t(33)
#define I2C0_SDA gpio_num_t(32)

// internal
#define I2C1_SCL gpio_num_t(22)
#define I2C1_SDA gpio_num_t(21)

// hat
#define I2C2_SCL gpio_num_t(26)
#define I2C2_SDA gpio_num_t(0)

//@NOTE WARNING https://docs.espressif.com/projects/esp-idf/en/v4.3.1/esp32/api-reference/peripherals/gpio.html?highlight=gpio#_CPPv416gpio_intr_enable10gpio_num_t
/* 
Please do not use the interrupt of GPIO36 and GPIO39 when using ADC or Wi-Fi with 
sleep mode enabled. Please refer to the comments of adc1_get_raw. Please refer 
to section 3.11 of ‘ECO_and_Workarounds_for_Bugs_in_ESP32’ for the description of this issue. 
As a workaround, call adc_power_acquire() in the app. 
This will result in higher power consumption (by ~1mA), but will remove the glitches 
on GPIO36 and GPIO39.
*/


// https://github.com/lvgl/lv_port_esp32
// https://docs.lvgl.io/latest/en/html/get-started/espressif.html

M5StickCPlusDevice::M5StickCPlusDevice(): ESP32Device((const char*)"(-) M5StickCPlus") {
    printf("%p %s Setup\n", this, this->name);
    //this->_period = 2000;
    printf("%p %s I2C Hy2.0-4P/GROVE port setup: $> i2cconfig --sda 32 --scl 33 --freq 400000\n", this, this->name);
    printf("%p %s I2C Internal port setup: $> i2cconfig --sda 21 --scl 22 --freq 400000\n", this, this->name);
    printf("%p %s I2C Hat port setup: $> i2cconfig --sda 0 --scl 26 --freq 400000\n", this, this->name);
    this->button0 = new ButtonDriver(BUTTON0_PIN);
    this->button1 = new ButtonDriver(BUTTON1_PIN);
    // AXP192 is connected to internal i2c
    this->axp192 = new AXP192Driver(I2C1_SDA, I2C1_SCL);
    printf("%p %s End Setup\n", this, this->name);

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
