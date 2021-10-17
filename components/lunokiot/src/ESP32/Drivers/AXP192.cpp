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
                                       Driver((const char*)"(-) AXP192", (unsigned long)PEK_BUTTON_POOL_TIME), 
                                        sda(sdagpio), scl(sclgpio) {
    printf("%p %s Setup sda=%d scl=%d\n", this, this->name, sda, scl);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
    
    //@NOTE dummy message, this code must be moved to i2cButtonDriver, bitmask is filter i2c response to get button bool status (pressed/released)
    printf("%p (-) Button Setup sda=%d scl=%d reg=0x%x bitmask=0x%x\n", this, sda, scl, REGISTERS::IRQ_STATUS_3, PEK_BUTTON::MASK);
}

bool AXP192Driver::Loop() {
    // @NOTE the following code must be ported to some kind of inheritance of Button, and Device must expose it as normal GPIO button


    // i2c Step 1, configure:
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
        this->_period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        return true;
    }

    // Step 2, install driver:
    res = i2c_driver_install(I2C_NUM_0, i2cConf.mode , I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_driver_install ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->_period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        return true;
    }

    // Step 3, get the data
    uint8_t buttonStatus = 0;
    const uint8_t registerToAsk = REGISTERS::IRQ_STATUS_3;

    res = i2c_master_write_read_device(I2C_NUM_0, I2C_ADDR_AXP192, &registerToAsk, 1, &buttonStatus, 1, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->_period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        i2c_driver_delete(I2C_NUM_0);
        return true;
    }


    //if ( PEK_BUTTON::RELEASED != buttonStatus ) {
    const uint8_t write_buf[2] = { REGISTERS::IRQ_STATUS_3, buttonStatus }; //PEK_BUTTON::MASK };
    res = i2c_master_write_to_device(I2C_NUM_0, I2C_ADDR_AXP192, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_master_write_to_device error: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->_period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        i2c_driver_delete(I2C_NUM_0);
        return true;
    }
    //}
    i2c_driver_delete(I2C_NUM_0);


    // logic button goes here:

    if ( buttonStatus ) {
        printf("@DEBUG (AXP192 i2c button: 0x%x OLD: 0x%x\n", buttonStatus, lastVal);
    }
    TickType_t thisEvent = xTaskGetTickCount();
    if ( buttonStatus != lastVal ) { // only if state of button changes

        printf("%p (-) Button(NA) ", this);
        printf("%s", buttonStatus?"Pressed":"Released" ); // inversed from Button GPIO
        
        if ( PEK_BUTTON::RELEASED == buttonStatus ) {
            // relased now
            TickType_t diffTime = thisEvent-lastEvent;
            printf(" %dms", diffTime);
        }
        
        if ( PEK_BUTTON::LONG == lastVal ) {
            printf(" long press event");
            this->_period = PEK_BUTTON_POOL_TIME; // released, low pooling
        } else if ( PEK_BUTTON::SHORT == lastVal ) {
            printf(" click event");
            this->_period = PEK_BUTTON_POOL_TIME; // released, low pooling
        } else {
            this->_period = PEK_BUTTON_SENSING_TIME; // button pressed, high resolution poll to get accurated time
        }
        printf("\n");
        lastEvent = thisEvent;
    }
    lastVal = buttonStatus;
    /*
    // the most important, ¿is the user pushing the POWER BUTTON?

    if ( PEK_BUTTON::LONG == buttonStatus ) { // do you like spagetti?
        if ( warnUserForPowerOff == true ) { // must warn user what are doing? (poweroff in 6 seconds!)
            //warnUserForPowerOff = false;
            TickType_t difftime = (xTaskGetTickCount() - lastEvent)/portTICK_RATE_MS;
            printf("\n\n /!\\ /!\\ /!\\ WARNING!!! RELEASE the BUTTON before %d seconds to AVOID POWERDOWN /!\\ /!\\ /!\\ \n\n\n", difftime/1000);
            this->_period = PEK_BUTTON_SENSING_TIME; // button pressed, high resolution poll to get accurated time
            fflush(stdout);
            lastVal = buttonStatus;
            return true;
        }
    } else if ( PEK_BUTTON::RELEASED == buttonStatus ) {
        if ( false == warnUserForPowerOff ) {
            printf("\nPOWEROFF procedure stopped due power button release :-)\n\n");
            warnUserForPowerOff = true;
        }
    }
    //printf("@DEBUG status: %d last: %d\n", buttonStatus, lastVal);
    */

    return true;
}
