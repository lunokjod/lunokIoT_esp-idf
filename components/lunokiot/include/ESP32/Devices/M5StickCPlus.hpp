#ifndef __LUNOKIOT__M5STICKCPLUS__
#define __LUNOKIOT__M5STICKCPLUS__


#include "LunokIoT.hpp"
#include "../Device.hpp"
#include "ESP32.hpp"
#include "../Drivers/Button.hpp"
#include "../Drivers/AXP192.hpp"
#include "../Drivers/LED.hpp"

namespace LunokIoT {

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


#define I2C_PORT I2C_NUM_0
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

//@TODO https://stackoverflow.com/questions/11711920/how-to-implement-multithread-safe-singleton-in-c11-without-using-mutex
    class M5StickCPlusDevice : public ESP32Device {
        public:
            M5StickCPlusDevice();
            AXP192Driver * axp192 = nullptr;
            ButtonDriver * button0 = nullptr;
            ButtonDriver * button1 = nullptr;
            LEDDriver * led = nullptr;
    };

}

#endif // __LUNOKIOT__M5STICKCPLUS__
