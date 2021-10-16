#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/AXP192.hpp"
#include "ESP32/Drivers/Button.hpp"
#include <hal/gpio_types.h>
#include "driver/i2c.h"
#include "ESP32/Drivers/I2C.hpp"
#include "base/I2CDatabase.hpp"

using namespace LunokIoT;



AXP192Driver::AXP192Driver(gpio_num_t sdagpio, gpio_num_t sclgpio): 
                                       Driver((const char*)"(-) AXP192", (unsigned long)200), 
                                        sda(sdagpio), scl(sclgpio) {
    printf("%p %s Setup sda=%d scl=%d\n", this, this->name, sda, scl);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
    

}

bool AXP192Driver::Loop() {
    // Step 1, configure:
    i2c_config_t i2cConf = { // --sda 21 --scl 22 --freq 400000
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (int)sda,
        .scl_io_num = (int)scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master {
            .clk_speed = I2C_MASTER_FREQ_HZ
        },
        .clk_flags = 0
    };
    esp_err_t res = i2c_param_config(I2C_NUM_0, &i2cConf);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_param_config ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->_period = 1000; // slowdown the next iteration
        return true;
    }

    // Step 2, install driver:
    res = i2c_driver_install(I2C_NUM_0, i2cConf.mode , I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_driver_install ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->_period = 1000; // slowdown the next iteration
        return true;
    }

    uint8_t buttonStatus = 0;
    uint8_t registerToAsk = (uint8_t)REGISTERS::IRQ_STATUS_3;

    res = i2c_master_write_read_device(I2C_NUM_0, I2C_ADDR_AXP192, &registerToAsk, 1, &buttonStatus, 1, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->_period = 1000; // slowdown the next iteration
        return true;
    }
    if ( PEK_BUTTON::RELEASED != buttonStatus ) {
        //printf("Result: %d\n", buttonStatus);
        uint8_t write_buf[2] = {REGISTERS::IRQ_STATUS_3,  0x3 };
        res = i2c_master_write_to_device(I2C_NUM_0, I2C_ADDR_AXP192, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_to_device error: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            this->_period = 1000; // slowdown the next iteration
            return true;
        }
    }
    res = i2c_driver_delete(I2C_NUM_0);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_driver_delete ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->_period = 1000; // slowdown the next iteration
        return true;
    }


    // @NOTE the following code must be ported to some kind of inheritance of Button, and Device must expose it as normal GPIO button
    
    if ( buttonStatus != lastVal ) {
        printf("%p %s(-1) ", this, "(-) Button");
        printf("%s ", buttonStatus?"Pressed":"Released" ); // inversed
        
        TickType_t thisEvent = xTaskGetTickCount();

        if ( PEK_BUTTON::RELEASED != buttonStatus ) {
            lastEvent = thisEvent;
            this->_period = 3; // button pressed, high resolution poll to get accurated time
        } else if ( PEK_BUTTON::RELEASED == buttonStatus ) {
            TickType_t diffTime = thisEvent-lastEvent;
            if ( diffTime > 0 ) {
                printf("%dms ", diffTime);

                if ( PEK_BUTTON::LONG == lastVal ) {
                    printf("long press");
                } else {
                    printf("click");
                }
                printf(" event");
            }
            this->_period = 200; // return to normal state
            lastEvent = thisEvent;
        }
        printf(" (AXP192 i2c button,  sda: %d scl: %d)\n", sda, scl);
        lastVal = buttonStatus;
    }
    this->_period = 200; // return to normal state
    return true;
}