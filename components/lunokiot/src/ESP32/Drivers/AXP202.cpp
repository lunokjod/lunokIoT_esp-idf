#include <driver/i2c.h>     // ESP32 i2c
#include <hal/gpio_types.h> // ESP32 gpio

#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/AXP202.hpp"
#include "ESP32/Drivers/Button.hpp"
#include "ESP32/Drivers/I2C.hpp"


using namespace LunokIoT;

void AXP202Driver::Init() {
    bool works = i2cHandler->GetI2CSession(port, frequency, sda, scl, address);
    if ( false == works ) { return; }

/*
    {
        i2cHandler->SetI2CChar(port, address, I2C_REGISTER::SHUTDOWN_BATTERY_CHGLED_CONTROL, 0x46);
        debug_printf("Enable batt detection");
    }
*/
   i2cHandler->FreeI2CSession(port);
}
AXP202Driver::AXP202Driver(I2CDriver *i2cHandler, i2c_port_t i2cport, uint32_t i2cfrequency, gpio_num_t i2csdagpio, gpio_num_t i2csclgpio, uint8_t i2caddress): 
                                       DriverBaseClass((const char*)"(-) AXP202", (unsigned long)PEK_BUTTON_POOL_TIME), 
                                        i2cHandler(i2cHandler), port(i2cport), frequency(i2cfrequency), sda(i2csdagpio), scl(i2csclgpio), address(i2caddress) {
    debug_printf("Setup (i2c port: %d, sda=%d scl=%d, addr:%d)", port, sda, scl, address);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
    
    this->Init();

    //@NOTE dummy message, this code must be moved to i2cButtonDriver, bitmask is filter i2c response to get button bool status (pressed/released)
    debug_printf("Button Setup (i2c reg=0x%x bitmask=0x%x)", I2C_REGISTER::IRQ_STATUS_3, PEK_BUTTON::MASK);
}

bool AXP202Driver::Loop() {
   bool works = i2cHandler->GetI2CSession(port, frequency, sda, scl, address);
    if ( true != works ) {
        return true; // try again next time
    }
    // Step 3, get the data
    uint8_t buttonStatus = 0;
    const uint8_t registerToAsk = I2C_REGISTER::IRQ_STATUS_3;

    esp_err_t res = i2c_master_write_read_device(I2C_NUM_0, I2C_ADDR_AXP202, &registerToAsk, 1, &buttonStatus, 1, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        i2c_driver_delete(I2C_NUM_0);
        return true;
    }


    // AXP202 irq ack
    const uint8_t write_buf[2] = { I2C_REGISTER::IRQ_STATUS_3, PEK_BUTTON::MASK };
    res = i2c_master_write_to_device(I2C_NUM_0, I2C_ADDR_AXP202, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_master_write_to_device error: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        i2c_driver_delete(I2C_NUM_0);
        return true;
    }
    i2cHandler->FreeI2CSession(port);


    // logic button goes here:
    TickType_t thisEvent = xTaskGetTickCount();
    if ( buttonStatus != lastVal ) { // only if state of button changes
        printf("%p (-) Button(-1) ", this);
        if ( buttonStatus == PEK_BUTTON::RELEASED) {
            printf("Released");
            // relased now
            TickType_t diffTime = thisEvent-lastEvent;
            printf(" %dms", diffTime);
        } else {
            printf("Pressed");
        }        
        
        if ( PEK_BUTTON::LONG == lastVal ) {
            printf(" long press event");
            this->period = PEK_BUTTON_POOL_TIME; // released, low pooling
        } else if ( PEK_BUTTON::SHORT == lastVal ) {
            printf(" click event");
            this->period = PEK_BUTTON_POOL_TIME; // released, low pooling
        } else {
            this->period = PEK_BUTTON_SENSING_TIME; // button pressed, high resolution poll to get accurated time
        }
        printf("\n");
        lastEvent = thisEvent;
    }
    lastVal = buttonStatus;

    // the most important, ¿is the user pushing the POWER BUTTON? send a warning!

    if ( PEK_BUTTON::LONG == buttonStatus ) { // do you like spagetti?
        if ( warnUserForPowerOff == true ) { // must warn user what are doing? (poweroff in 6 seconds!)
            printf("\n\n /!\\ /!\\ /!\\ WARNING!!! RELEASE the BUTTON before 6 seconds to AVOID POWERDOWN /!\\ /!\\ /!\\ \n\n\n");
            this->period = PEK_BUTTON_SENSING_TIME; // button pressed, high resolution poll to get accurated time
            fflush(stdout);
            return true;
        }
    } else if ( PEK_BUTTON::RELEASED == buttonStatus ) {
        if ( false == warnUserForPowerOff ) {
            printf("\nPOWEROFF procedure stopped due power button release :-)\n\n");
            warnUserForPowerOff = true;
        }
    }
    return true;
}
