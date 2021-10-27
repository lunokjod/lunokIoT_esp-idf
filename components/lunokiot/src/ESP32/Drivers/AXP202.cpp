#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>     // ESP32 i2c
#include <hal/gpio_types.h> // ESP32 gpio
#include <driver/gpio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/AXP202.hpp"
#include "ESP32/Drivers/Button.hpp"
#include "ESP32/Drivers/I2C.hpp"

// https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/docs/watch_2020_v3.md
// volatile 
bool axp202Interrupt = false;
static void IRAM_ATTR gpioHandler(void* arg) {
    axp202Interrupt = true;
}
static void setupIRQ(void *arg){
    debug_printf("setupIRQ");

    //LunokIoT::AXP202Driver *instance = static_cast<LunokIoT::AXP202Driver*>(arg);
    //instance->Init();
    
    gpio_config_t io_conf = {};
    //ESP_ERROR_CHECK(gpio_set_intr_type(AXP202_INT, GPIO_INTR_ANYEDGE));
    io_conf.intr_type = gpio_int_type_t(GPIO_INTR_ANYEDGE);
    io_conf.mode = gpio_mode_t(GPIO_MODE_INPUT);
    io_conf.pull_down_en = gpio_pulldown_t(false);
    io_conf.pull_up_en = gpio_pullup_t(false);
    io_conf.pin_bit_mask = (1ULL<<AXP202_INT);
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(AXP202_INT, gpioHandler, (void*)nullptr));

    vTaskDelete(NULL);
}

using namespace LunokIoT;

void AXP202Driver::Init() {
 

    bool works = i2cHandler->GetI2CSession(port, frequency, sda, scl, address);
    if ( false == works ) { return; }
    
    // enable all status registers
    /* bits: [X]=RESERVED,                 [1]=VBUS LOW,             [2]=VBUS REMOVED,      [3]=VBUS CONNECTED,      [4]=VBUS OVERVOLTAGE,    [5]=ACIN REMOVED,  [6]=ACIN CONNECTED,      [7]=ACIN OVERVOLTAGE  */    
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_ENABLE_1, 0b01111111);
    /* bits: [0]=BATT LOW TEMP,            [1]=BATT OVERHEAT,        [2]=BATT CHARGED,      [3]=BATT CHARGING,       [4]=BATT EXIT ACTIVATE,  [5]=BATT ACTIVATE, [6]=BATT REMOVED,        [7]=BATT CONNECTED    */
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_ENABLE_2, 0b11111111);
    /* bits: [0]=PEK LONG,                 [1]=PEK SHORT,            [2]=LDO3 UNDERVOLTAGE, [3]=DC-DC3 UNDERVOLTAGE, [4]=DC-DC2 UNDERVOLTAGE, [X]=RESERVED,      [6]=CHARGE UNDERVOLTAGE, [7]=INTERNAL OVERHEAT */
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_ENABLE_3, 0b11111011);
    /* bits: [0]=AXP UNDERVOLTAGE LEVEL 2, [1]=UNDERVOLTAGE LEVEL 1, [2]=VBUS SESSION END,  [3]=VBUS SESION A/B,     [4]=VBUS INVALID,        [5]=VBUS VALID,    [6]=N_OE SHUTDOWN,       [7]=N_OE STARTUP      */
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_ENABLE_4, 0b11111111);
    /* bits: [0]=GPIO0 INPUT,              [1]=GPIO1 INPUT,          [2]=GPIO2 INPUT,       [3]=GPIO3 INPUT,         [X]=RESERVED,            [5]=PEK PRESS,     [6]=PEK RELEASED,        [7]=TIMER TIMEOUT     */
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_ENABLE_5, 0b11110111);

   i2cHandler->FreeI2CSession(port);
    // @TODO is possible to do a interrupt handling?
}

AXP202Driver::AXP202Driver(I2CDriver *i2cHandler, i2c_port_t i2cport, uint32_t i2cfrequency, gpio_num_t i2csdagpio, gpio_num_t i2csclgpio, uint8_t i2caddress): 
                                       DriverBaseClass((const char*)"(-) AXP202", (unsigned long)PEK_BUTTON_POOL_TIME), 
                                        i2cHandler(i2cHandler), port(i2cport), frequency(i2cfrequency), sda(i2csdagpio), scl(i2csclgpio), address(i2caddress) {
    debug_printf("Setup (i2c port: %d, sda=%d scl=%d, addr:%d)", port, sda, scl, address);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
    
    xTaskCreate(setupIRQ, "setup irq", 1024 * 2, (void *)this, 10, NULL); // ugly trick to pin the interrupt to core
    this->Init();
    this->ReadStatus();
    this->Clearbits();

    //@NOTE dummy message, this code must be moved to i2cButtonDriver, bitmask is filter i2c response to get button bool status (pressed/released)
    //debug_printf("Button Setup (i2c reg=0x%x bitmask=0x%x)", I2C_REGISTER::IRQ_STATUS_3, PEK_BUTTON::MASK);
    
    this->period = 300;  // time between irq detection
}

// here the last copy of loop
uint8_t IRQ_STATUS_1_CACHE = 0x0;
uint8_t IRQ_STATUS_2_CACHE = 0x0;
uint8_t IRQ_STATUS_3_CACHE = 0x0;
uint8_t IRQ_STATUS_4_CACHE = 0x0;
uint8_t IRQ_STATUS_5_CACHE = 0x0;

bool AXP202Driver::Clearbits() {
    // talk with AXP202
    bool works = i2cHandler->GetI2CSession(port, frequency, sda, scl, address);
    if ( true != works ) {
        debug_printferror("Woah! unable to start i2c session");
        return false; // try again next time
    }
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_1, 0xff);
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_2, 0xff);
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_3, 0xff);
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_4, 0xff);
    i2cHandler->SetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_5, 0xff);

    // free i2c
    i2cHandler->FreeI2CSession(port);
    return true;
}
bool AXP202Driver::ReadStatus() {
    // talk with AXP202
    bool works = i2cHandler->GetI2CSession(port, frequency, sda, scl, address);
    if ( true != works ) {
        debug_printferror("Unable to start i2c session");
        return true; // try again next time
    }
    // get status1
    uint8_t status1 = 0x0;
    works = i2cHandler->GetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_1, status1);
    if ( works ) {
        if ( status1 != IRQ_STATUS_1_CACHE ) {
            debug_printf("IRQ_STATUS_1: 0x%x", status1);
            IRQ_STATUS_1_CACHE = status1; // update cache
            
        }
    }
    // get status2
    uint8_t status2 = 0x0;
    works = i2cHandler->GetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_2, status2);
    if ( works ) {
        if ( status2 != IRQ_STATUS_2_CACHE ) {
            debug_printf("IRQ_STATUS_2: 0x%x", status2);
            IRQ_STATUS_2_CACHE = status2; // update cache
            
        }
    }
    // get status3
    uint8_t status3 = 0x0;
    works = i2cHandler->GetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_3, status3);
    if ( works ) {
        if ( status3 != IRQ_STATUS_3_CACHE ) {
            debug_printf("IRQ_STATUS_3: 0x%x", status3);
            IRQ_STATUS_3_CACHE = status3; // update cache
        }
    }
    // get status4
    uint8_t status4 = 0x0;
    works = i2cHandler->GetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_4, status4);
    if ( works ) {
        if ( status4 != IRQ_STATUS_4_CACHE ) {
            debug_printf("IRQ_STATUS_4: 0x%x", status4);
            IRQ_STATUS_4_CACHE = status4; // update cache
        }
    }
    // get status5
    uint8_t status5 = 0x0;
    works = i2cHandler->GetI2CChar(port, address, I2C_REGISTER::IRQ_STATUS_5, status5);
    if ( works ) {
        if ( status5 != IRQ_STATUS_5_CACHE ) {
            debug_printf("IRQ_STATUS_5: 0x%x", status5);
            IRQ_STATUS_5_CACHE = status5; // update cache
        }
    }
    // free i2c
    i2cHandler->FreeI2CSession(port);
    return true;
}

// I hate pooling
bool AXP202Driver::Loop() {
    
    // no interrupt triggered?, get out!
    if ( false == axp202Interrupt ) { 
        return true;
    }
    debug_printf("INT: %s", axp202Interrupt?"true":"false");

    this->ReadStatus();
    this->Clearbits();
    //this->Init();

    axp202Interrupt = false;
    return true;
/*

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
    */
}
