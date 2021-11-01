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
// code/lilygo/TTGO_TWatch_Library/src/drive/axp/axp20x.cpp
// code/lilygo/TTGO_TWatch_Library/src/TTGO.h:1346
// https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/docs/watch_2020_v3.md
// volatile 
bool axp202Interrupt = false;
bool axp202Interrupt_DCDC3_voltage_high = false; // IRQ20

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

    vTaskDelete(NULL); // kill myself, the only needed data of this task are the core pinned to
}

using namespace LunokIoT;


void AXP202Driver::DumpRegisters() {
    debug_printf("AXP202 register dump:");

    lunokiot_i2c_channel_descriptor_t i2cDescriptor = {};
    bool works = i2cHandler->GetSession(frequency, sda, scl, i2cDescriptor);
    if ( not works ) {
        debug_printferror("Unable to get i2c session");
        return;
    }

    // Power Input Status register
    uint8_t status = 0x0;
    works = i2cHandler->GetChar(i2cDescriptor, address, I2C_REGISTER::STATUS, status);
    if ( !works ) {
        i2cHandler->FreeSession(i2cDescriptor);
        return;
    }
    bool BOOT_SOURCE = status;
    bool SHORT_IN_ACIN_VBUS = status >> 1;
    bool BATTERY_CURRENT_DIRECTION = status >> 2;
    bool VBUS_ABOVE_VHOLD = status >> 3;
    bool VBUS_USABLE = status >> 4;
    bool VBUS_PRESENCE = status >> 5;
    bool ACIN_USABLE = status >> 6;
    bool ACIN_PRESENCE = status >> 7;
    
    debug_printf("Power Input Status register (got 0x%x)", status);
    debug_printf("   Boot source: %s", BOOT_SOURCE?"other":"ACIN/VBUS");
    debug_printf("   Shortcircuit on ACIN/VBUS: %s", SHORT_IN_ACIN_VBUS?"yes":"no");
    debug_printf("   Battery voltage direction: %s", BATTERY_CURRENT_DIRECTION?"in":"out");
    debug_printf("   VBUS above VHOLD: %s", VBUS_ABOVE_VHOLD?"true":"false");
    debug_printf("   VBUS usable: %s", VBUS_USABLE?"true":"false");
    debug_printf("   VBUS presence: %s", VBUS_PRESENCE?"true":"false");
    debug_printf("   ACIN usable: %s", ACIN_USABLE?"true":"false");
    debug_printf("   ACIN presence: %s", ACIN_PRESENCE?"true":"false");

    
//POWER_MODE_CHRG
    uint8_t powerMode = 0x0;
    works = i2cHandler->GetChar(i2cDescriptor, address, I2C_REGISTER::POWER_MODE_CHRG, powerMode);
    if ( !works ) {
        i2cHandler->FreeSession(i2cDescriptor);
        return;
    }
    // 0~1 reserved
    bool LOWER_CHARGING_CURRENT = powerMode >> 2;
    bool BATTERY_ACTIVATE = powerMode >> 3;
    // 4 reserved
    bool BATTERY_INSTALLED = powerMode >> 5;
    bool BATTERY_CHARGING = powerMode >> 6;
    bool AXP_OVERTEMP = powerMode >> 7;
    debug_printf("Power Working Mode and Charge Status Indication (got 0x%x)", powerMode);
    debug_printf("   Low charging current: %s", LOWER_CHARGING_CURRENT?"yes":"no");
    debug_printf("   Battery activate: %s", BATTERY_ACTIVATE?"yes":"no");
    debug_printf("   Battery installed: %s", BATTERY_INSTALLED?"yes":"no");
    debug_printf("   Battery charging: %s", BATTERY_CHARGING?"yes":"no");
    debug_printf("   AXP temperature status: %s", AXP_OVERTEMP?"OVER TEMPERATURE!":"normal");

//OTG_VBUS
    uint8_t otgVBUS = 0x0;
    works = i2cHandler->GetChar(i2cDescriptor, address, I2C_REGISTER::OTG_VBUS, otgVBUS);
    if ( !works ) {
        i2cHandler->FreeSession(i2cDescriptor);
        return;
    }
    bool SESSION_END = otgVBUS;
    bool VBUS_AB_SESSION = otgVBUS >> 1;
    bool VBUS_VALID = otgVBUS >> 2;
    debug_printf("USB OTG VBUS Status Indication (got 0x%x)", otgVBUS);
    debug_printf("   Session end: %s", SESSION_END?"valid":"invalid");
    debug_printf("   VBUS A/B session: %s", VBUS_AB_SESSION?"yes":"no");
    debug_printf("   VBUS valid: %s", VBUS_VALID?"yes":"no");

//Coulomb Counter Control
    uint8_t coloumbControl = 0x0;
    works = i2cHandler->GetChar(i2cDescriptor, address, I2C_REGISTER::COLOUMB_COUNTER, coloumbControl);
    if ( !works ) {
        i2cHandler->FreeSession(i2cDescriptor);
        return;
    }
    bool DECRYPTION_DONE = coloumbControl;
    bool DECRYPTION_START = coloumbControl >> 1;
    bool PAUSE_COLOUMB_COUNTER = coloumbControl >> 6;
    bool ENABLE_COLOUMB_COUNTER = coloumbControl >> 7;
    debug_printf("Coloumb counter (got 0x%x)", coloumbControl);
    debug_printf("   Decryption done: %s", DECRYPTION_DONE?"yes":"no");
    debug_printf("   Decryption in progress: %s", DECRYPTION_START?"yes":"no");
    debug_printf("   Counter paused: %s", PAUSE_COLOUMB_COUNTER?"yes":"no");
    debug_printf("   Counter enabled: %s", ENABLE_COLOUMB_COUNTER?"yes":"no");

// FUEL_GAUGE=(0xB9) // bits: [6~0]=charge percentage, [7]=work mode (0=normal, 1=suspended)
   uint8_t fuelGauge = 0x0;
    works = i2cHandler->GetChar(i2cDescriptor, address, I2C_REGISTER::FUEL_GAUGE, fuelGauge);
    if ( !works ) {
        i2cHandler->FreeSession(i2cDescriptor);
        return;
    }
    bool CHARGE_PC = fuelGauge & 0b01111111; // remove last bit to get the value range [0~6]
    bool WORK_MODE = fuelGauge >> 7;
    debug_printf("Fuel Gauge (got 0x%x)", fuelGauge);
    debug_printf("   Percent: %d%%", CHARGE_PC);
    debug_printf("   Work mode: %s", WORK_MODE?"Suspended":"Normal");

    i2cHandler->FreeSession(i2cDescriptor);
}
void AXP202Driver::Init() {
 
    lunokiot_i2c_channel_descriptor_t i2cDescriptor = {};
    bool works = i2cHandler->GetSession(frequency, sda, scl, i2cDescriptor);
    
    //i2cHandler->GetI2CSession(port, frequency, sda, scl, address);
    if ( false == works ) { 
        debug_printferror("Unable to get i2c session!");
        return;
    }
    
    // enable all status registers and interrupts
    /* bits: [X]=RESERVED,                 [1]=VBUS LOW,             [2]=VBUS REMOVED,      [3]=VBUS CONNECTED,      [4]=VBUS OVERVOLTAGE,    [5]=ACIN REMOVED,  [6]=ACIN CONNECTED,      [7]=ACIN OVERVOLTAGE  */
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_ENABLE_1, 0b01111111);
    //i2cHandler->SetI2CChar(i2cDescriptor.port, address, I2C_REGISTER::IRQ_ENABLE_1, 0b01111111);
    /* bits: [0]=BATT LOW TEMP,            [1]=BATT OVERHEAT,        [2]=BATT CHARGED,      [3]=BATT CHARGING,       [4]=BATT EXIT ACTIVATE,  [5]=BATT ACTIVATE, [6]=BATT REMOVED,        [7]=BATT CONNECTED    */
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_ENABLE_2, 0b11111111);
    /* bits: [0]=PEK LONG,                 [1]=PEK SHORT,            [2]=LDO3 UNDERVOLTAGE, [3]=DC-DC3 UNDERVOLTAGE, [4]=DC-DC2 UNDERVOLTAGE, [X]=RESERVED,      [6]=CHARGE UNDERVOLTAGE, [7]=INTERNAL OVERHEAT */
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_ENABLE_3, 0b11111011);
    /* bits: [0]=AXP UNDERVOLTAGE LEVEL 2, [1]=UNDERVOLTAGE LEVEL 1, [2]=VBUS SESSION END,  [3]=VBUS SESION A/B,     [4]=VBUS INVALID,        [5]=VBUS VALID,    [6]=N_OE SHUTDOWN,       [7]=N_OE STARTUP      */
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_ENABLE_4, 0b11111111);
    /* bits: [0]=GPIO0 INPUT,              [1]=GPIO1 INPUT,          [2]=GPIO2 INPUT,       [3]=GPIO3 INPUT,         [X]=RESERVED,            [5]=PEK PRESS,     [6]=PEK RELEASED,        [7]=TIMER TIMEOUT     */
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_ENABLE_5, 0b11110111);

    //@DEBUG here goes the timer test
    debug_printf("@DEBUG REMOVE THIS TEST (Aka TIMER TEST) setted to trigger in 1 minute on the future");
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::TIMER_CONTROL, 0b00000010); // send interrupt in one minute
    i2cHandler->FreeSession(i2cDescriptor);

    this->DumpRegisters();
    
}

AXP202Driver::AXP202Driver(I2CDriver *i2cHandler, i2c_port_t i2cport, uint32_t i2cfrequency, gpio_num_t i2csdagpio, gpio_num_t i2csclgpio, uint8_t i2caddress): 
                                       DriverBaseClass((const char*)"(-) AXP202", (unsigned long)PEK_BUTTON_POOL_TIME), 
                                        i2cHandler(i2cHandler), port(i2cport), frequency(i2cfrequency), sda(i2csdagpio), scl(i2csclgpio), address(i2caddress) {
    debug_printf("Setup (i2c port: %d, sda=%d scl=%d, addr:%d)", port, sda, scl, address);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
    // install irq handler for AXP202
    xTaskCreate(setupIRQ, "setup irq", 1024 * 2, (void *)this, 10, NULL); // ugly trick to pin the interrupt to core
    this->ReadStatus();
    this->Clearbits();
    this->Init();

    //@NOTE dummy message, this code must be moved to i2cButtonDriver, bitmask is filter i2c response to get button bool status (pressed/released)
    //debug_printf("Button Setup (i2c reg=0x%x bitmask=0x%x)", I2C_REGISTER::IRQ_STATUS_3, PEK_BUTTON::MASK);
    
    this->period = 300;  // time between irq event parse loop
}

// here the last copy of loop
uint8_t IRQ_STATUS_CACHE_LAST[] = { 0x0, 0x0, 0x0, 0x0, 0x0 };
uint8_t IRQ_STATUS_CACHE_CURRENT[] = { 0x0, 0x0, 0x0, 0x0, 0x0 };
/*
uint8_t IRQ_STATUS_2_CACHE = 0x0;
uint8_t IRQ_STATUS_3_CACHE = 0x0;
uint8_t IRQ_STATUS_4_CACHE = 0x0;
uint8_t IRQ_STATUS_5_CACHE = 0x0;

uint8_t IRQ_STATUS_1_CACHE = 0x0;
uint8_t IRQ_STATUS_2_CACHE = 0x0;
uint8_t IRQ_STATUS_3_CACHE = 0x0;
uint8_t IRQ_STATUS_4_CACHE = 0x0;
uint8_t IRQ_STATUS_5_CACHE = 0x0;
*/

bool AXP202Driver::Clearbits() {
    lunokiot_i2c_channel_descriptor_t i2cDescriptor = {};
    bool works = i2cHandler->GetSession(frequency, sda, scl, i2cDescriptor);
    if ( true != works ) {
        debug_printferror("Unable to start i2c session");
        return false; // try again later
    }
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_1, 0b01111111); // zero to reserved
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_2, 0xff);
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_3, 0b11111011); // zero to reserved
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_4, 0xff);
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_5, 0xff);

    /*
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_1, IRQ_STATUS_CACHE_CURRENT[0]);
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_2, IRQ_STATUS_CACHE_CURRENT[1]);
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_3, IRQ_STATUS_CACHE_CURRENT[2]);
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_4, IRQ_STATUS_CACHE_CURRENT[3]);
    i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::IRQ_STATUS_5, IRQ_STATUS_CACHE_CURRENT[4]);
    */

    // free i2c
    
    //i2cHandler->FreeI2CSession(port);
    i2cHandler->FreeSession(i2cDescriptor);
    return true;
}
void AXP202Driver::DescribeStatus(uint8_t status[5]=IRQ_STATUS_CACHE_CURRENT) { // https://www.tutorialspoint.com/cplusplus/cpp_bitwise_operators.htm
    // print bit offset table
//    printf("\nSTATUS  : [0][1][2][3][4][5][6][7]\n");
    printf("          [0][1][2][3][4][5][6][7]\n");
    /*
    if ( pdTRUE != xSemaphoreTake(statusMutex, portMAX_DELAY) ) {
        debug_printferror("Unable to get status mutex");
        return;
    }*/
    //IRQ_STATUS_1_CACHE
    //bool IRQ_ENABLE_1_BIT0_RESERVED = false;
    bool IRQ_ENABLE_1_BIT1_VBUS_LOW = status[0] >> 1;
    bool IRQ_ENABLE_1_BIT2_VBUS_REMOVED = status[0] >> 2;
    bool IRQ_ENABLE_1_BIT3_VBUS_CONNECTED = status[0] >> 3;
    bool IRQ_ENABLE_1_BIT4_VBUS_OVERVOLTAGE = status[0] >> 4;
    bool IRQ_ENABLE_1_BIT5_ACIN_REMOVED = status[0] >> 5;
    bool IRQ_ENABLE_1_BIT6_ACIN_CONNECTED = status[0] >> 6;
    bool IRQ_ENABLE_1_BIT7_ACIN_OVERVOLTAGE = status[0] >> 7;
    //IRQ_STATUS_2_CACHE
    bool IRQ_ENABLE_2_BIT0_BATT_LOW_TEMP = status[1];
    bool IRQ_ENABLE_2_BIT1_BATT_OVERHEAT = status[1] >> 1;
    bool IRQ_ENABLE_2_BIT2_BATT_CHARGED = status[1] >> 2;
    bool IRQ_ENABLE_2_BIT3_BATT_CHARGING = status[1] >> 3;
    bool IRQ_ENABLE_2_BIT4_BATT_EXIT_ACTIVATE =  status[1] >> 4;
    bool IRQ_ENABLE_2_BIT5_BATT_ACTIVATE =  status[1] >> 5;
    bool IRQ_ENABLE_2_BIT6_BATT_REMOVED =  status[1] >> 6;
    bool IRQ_ENABLE_2_BIT6_BATT_CONNECTED =  status[1] >> 7;
    //IRQ_STATUS_3_CACHE
    bool IRQ_ENABLE_3_BIT0_PEK_LONG = status[2];
    bool IRQ_ENABLE_3_BIT1_PEK_SHORT = status[2] >> 1;
    bool IRQ_ENABLE_3_BIT2_LDO3_UNDERVOLTAGE =  status[2] >> 2;
    bool IRQ_ENABLE_3_BIT3_DC_DC3_UNDERVOLTAGE =  status[2] >> 3;
    bool IRQ_ENABLE_3_BIT4_DC_DC2_UNDERVOLTAGE =  status[2] >> 4;
    //bool IRQ_ENABLE_3_BIT5_RESERVED = false;
    bool IRQ_ENABLE_3_BIT6_CHARGE_UNDERVOLTAGE =  status[2] >> 6;
    bool IRQ_ENABLE_3_BIT7_INTERNAL_OVERHEAT =  status[2] >> 7;
    //IRQ_STATUS_4_CACHE
    bool IRQ_ENABLE_4_BIT0_AXP_UNDERVOLTAGE_LEVEL2 = status[3];
    bool IRQ_ENABLE_4_BIT1_AXP_UNDERVOLTAGE_LEVEL1 = status[3] >> 1;
    bool IRQ_ENABLE_4_BIT2_VBUS_SESSION_END = status[3] >> 2;
    bool IRQ_ENABLE_4_BIT3_VBUS_SESSION_AB = status[3] >> 3;
    bool IRQ_ENABLE_4_BIT4_VBUS_INVALID = status[3] >> 4;
    bool IRQ_ENABLE_4_BIT5_VBUS_VALID = status[3] >> 5;
    bool IRQ_ENABLE_4_BIT6_N_OE_SHUTDOWN = status[3] >> 6;
    bool IRQ_ENABLE_4_BIT7_N_OE_STARTUP = status[3] >> 7;
    //IRQ_STATUS_5_CACHE
    bool IRQ_ENABLE_5_BIT0_GPIO0_INPUT = status[4];
    bool IRQ_ENABLE_5_BIT1_GPIO1_INPUT = status[4] >>1;
    bool IRQ_ENABLE_5_BIT2_GPIO2_INPUT = status[4] >>2;
    bool IRQ_ENABLE_5_BIT3_GPIO3_INPUT = status[4] >>3;
    //bool IRQ_ENABLE_5_BIT4_RESERVED = false;
    bool IRQ_ENABLE_5_BIT5_PEK_PRESS =  status[4] >>5;
    bool IRQ_ENABLE_5_BIT6_PEK_RELEASED =  status[4] >>6;
    bool IRQ_ENABLE_5_BIT7_TIMER_TIMEOUT =  status[4] >>7;
    //xSemaphoreGive(statusMutex);

    printf("STATUS 1: ");
    //bool IRQ_ENABLE_1_BIT0_RESERVED = false;
    printf("%s[R]%s", TERM_FG_GREY, TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_1_BIT1_VBUS_LOW?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_1_BIT1_VBUS_LOW?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_1_BIT2_VBUS_REMOVED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_1_BIT2_VBUS_REMOVED?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_1_BIT3_VBUS_CONNECTED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_1_BIT3_VBUS_CONNECTED?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_1_BIT4_VBUS_OVERVOLTAGE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_1_BIT4_VBUS_OVERVOLTAGE?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_1_BIT5_ACIN_REMOVED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_1_BIT5_ACIN_REMOVED?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_1_BIT6_ACIN_CONNECTED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_1_BIT6_ACIN_CONNECTED?"X":" ", TERM_RESET);
    printf("%s[%s]%s ", IRQ_ENABLE_1_BIT7_ACIN_OVERVOLTAGE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_1_BIT7_ACIN_OVERVOLTAGE?"X":" ", TERM_RESET);

    printf("%sreserved%s, ", TERM_FG_GREY, TERM_RESET);
    printf("%svbus low%s, ",IRQ_ENABLE_1_BIT1_VBUS_LOW?TERM_FG_GREEN:TERM_RESET , TERM_RESET);
    printf("%svbus removed%s, ",IRQ_ENABLE_1_BIT2_VBUS_REMOVED?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%svbus removed%s, ",IRQ_ENABLE_1_BIT2_VBUS_REMOVED?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%svbus connected%s, ",IRQ_ENABLE_1_BIT3_VBUS_CONNECTED?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%svbus overvoltage%s, ",IRQ_ENABLE_1_BIT4_VBUS_OVERVOLTAGE?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%svbus removed%s, ",IRQ_ENABLE_1_BIT5_ACIN_REMOVED?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%svbus connected%s, ",IRQ_ENABLE_1_BIT6_ACIN_CONNECTED?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%svbus overvoltage%s",IRQ_ENABLE_1_BIT7_ACIN_OVERVOLTAGE?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("\n");

    printf("STATUS 2: ");
    printf("%s[%s]%s", IRQ_ENABLE_2_BIT0_BATT_LOW_TEMP?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT0_BATT_LOW_TEMP?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_2_BIT1_BATT_OVERHEAT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT1_BATT_OVERHEAT?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_2_BIT2_BATT_CHARGED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT2_BATT_CHARGED?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_2_BIT3_BATT_CHARGING?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT3_BATT_CHARGING?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_2_BIT4_BATT_EXIT_ACTIVATE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT4_BATT_EXIT_ACTIVATE?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_2_BIT5_BATT_ACTIVATE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT5_BATT_ACTIVATE?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_2_BIT6_BATT_REMOVED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT6_BATT_REMOVED?"X":" ", TERM_RESET);
    printf("%s[%s]%s ", IRQ_ENABLE_2_BIT6_BATT_CONNECTED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_2_BIT6_BATT_CONNECTED?"X":" ", TERM_RESET);

    printf("%sbatt low temp%s, ",IRQ_ENABLE_2_BIT0_BATT_LOW_TEMP?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("%sbatt overheat%s, ",IRQ_ENABLE_2_BIT1_BATT_OVERHEAT?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("%sbatt charged%s, ",IRQ_ENABLE_2_BIT2_BATT_CHARGED?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("%sbatt charging%s, ",IRQ_ENABLE_2_BIT3_BATT_CHARGING?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("%sbatt exit activate%s, ",IRQ_ENABLE_2_BIT4_BATT_EXIT_ACTIVATE?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("%sbatt activate%s, ",IRQ_ENABLE_2_BIT5_BATT_ACTIVATE?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("%sbatt removed%s, ",IRQ_ENABLE_2_BIT6_BATT_REMOVED?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("%sbatt connected%s",IRQ_ENABLE_2_BIT6_BATT_CONNECTED?TERM_FG_GREEN:TERM_RESET,TERM_RESET);
    printf("\n");

    printf("STATUS 3: ");
    printf("%s[%s]%s", IRQ_ENABLE_3_BIT0_PEK_LONG?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_3_BIT0_PEK_LONG?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_3_BIT1_PEK_SHORT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_3_BIT1_PEK_SHORT?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_3_BIT2_LDO3_UNDERVOLTAGE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_3_BIT2_LDO3_UNDERVOLTAGE?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_3_BIT3_DC_DC3_UNDERVOLTAGE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_3_BIT3_DC_DC3_UNDERVOLTAGE?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_3_BIT4_DC_DC2_UNDERVOLTAGE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_3_BIT4_DC_DC2_UNDERVOLTAGE?"X":" ", TERM_RESET);
    printf("%s[R]%s", TERM_FG_GREY, TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_3_BIT6_CHARGE_UNDERVOLTAGE?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_3_BIT6_CHARGE_UNDERVOLTAGE?"X":" ", TERM_RESET);
    printf("%s[%s]%s ", IRQ_ENABLE_3_BIT7_INTERNAL_OVERHEAT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_3_BIT7_INTERNAL_OVERHEAT?"X":" ", TERM_RESET);


    printf("%spek_long%s, ",IRQ_ENABLE_3_BIT0_PEK_LONG?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%spek_short%s, ",IRQ_ENABLE_3_BIT1_PEK_SHORT?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sLDO3 undervoltage%s, ",IRQ_ENABLE_3_BIT2_LDO3_UNDERVOLTAGE?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sDC-DC3 undervoltage%s, ",IRQ_ENABLE_3_BIT3_DC_DC3_UNDERVOLTAGE?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sDC-DC2 undervoltafe%s, ",IRQ_ENABLE_3_BIT4_DC_DC2_UNDERVOLTAGE?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sreserved%s, ", TERM_FG_GREY, TERM_RESET);
    //bool IRQ_ENABLE_3_BIT5_RESERVED = false;
    printf("%scharge undervoltage%s, ",IRQ_ENABLE_3_BIT6_CHARGE_UNDERVOLTAGE?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sinternal overheat%s",IRQ_ENABLE_3_BIT7_INTERNAL_OVERHEAT?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("\n");

    printf("STATUS 4: ");
    printf("%s[%s]%s", IRQ_ENABLE_4_BIT0_AXP_UNDERVOLTAGE_LEVEL2?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT0_AXP_UNDERVOLTAGE_LEVEL2?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_4_BIT1_AXP_UNDERVOLTAGE_LEVEL1?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT1_AXP_UNDERVOLTAGE_LEVEL1?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_4_BIT2_VBUS_SESSION_END?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT2_VBUS_SESSION_END?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_4_BIT3_VBUS_SESSION_AB?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT3_VBUS_SESSION_AB?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_4_BIT4_VBUS_INVALID?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT4_VBUS_INVALID?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_4_BIT5_VBUS_VALID?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT5_VBUS_VALID?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_4_BIT6_N_OE_SHUTDOWN?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT6_N_OE_SHUTDOWN?"X":" ", TERM_RESET);
    printf("%s[%s]%s ", IRQ_ENABLE_4_BIT7_N_OE_STARTUP?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_4_BIT7_N_OE_STARTUP?"X":" ", TERM_RESET);

    printf("%sAXP undervoltage level2%s, ",IRQ_ENABLE_4_BIT0_AXP_UNDERVOLTAGE_LEVEL2?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sAXP undervoltage level1%s, ",IRQ_ENABLE_4_BIT1_AXP_UNDERVOLTAGE_LEVEL1?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sVBus session end%s, ",IRQ_ENABLE_4_BIT2_VBUS_SESSION_END?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sVBus session A/B%s, ",IRQ_ENABLE_4_BIT3_VBUS_SESSION_AB?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sVBus invalid%s, ",IRQ_ENABLE_4_BIT4_VBUS_INVALID?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sVBus valid%s, ",IRQ_ENABLE_4_BIT5_VBUS_VALID?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sN_OE shutdown%s, ",IRQ_ENABLE_4_BIT6_N_OE_SHUTDOWN?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sN_OE startup%s\n",IRQ_ENABLE_4_BIT7_N_OE_STARTUP?TERM_FG_GREEN:TERM_RESET, TERM_RESET);

    printf("STATUS 5: ");
    printf("%s[%s]%s", IRQ_ENABLE_5_BIT0_GPIO0_INPUT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT0_GPIO0_INPUT?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_5_BIT1_GPIO1_INPUT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT1_GPIO1_INPUT?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_5_BIT2_GPIO2_INPUT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT2_GPIO2_INPUT?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_5_BIT2_GPIO2_INPUT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT2_GPIO2_INPUT?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_5_BIT3_GPIO3_INPUT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT3_GPIO3_INPUT?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_5_BIT5_PEK_PRESS?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT5_PEK_PRESS?"X":" ", TERM_RESET);
    printf("%s[%s]%s", IRQ_ENABLE_5_BIT6_PEK_RELEASED?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT6_PEK_RELEASED?"X":" ", TERM_RESET);
    printf("%s[%s]%s ", IRQ_ENABLE_5_BIT7_TIMER_TIMEOUT?TERM_FG_GREEN:TERM_FG_RED ,IRQ_ENABLE_5_BIT7_TIMER_TIMEOUT?"X":" ", TERM_RESET);

    printf("%sgpio0 input%s, ",IRQ_ENABLE_5_BIT0_GPIO0_INPUT?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sgpio1 input%s, ",IRQ_ENABLE_5_BIT1_GPIO1_INPUT?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sgpio2 input%s, ",IRQ_ENABLE_5_BIT2_GPIO2_INPUT?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%sgpio3 input%s, ",IRQ_ENABLE_5_BIT3_GPIO3_INPUT?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    //bool IRQ_ENABLE_5_BIT4_RESERVED = false;
    printf("%spek press%s, ",IRQ_ENABLE_5_BIT5_PEK_PRESS?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%spek released%s, ",IRQ_ENABLE_5_BIT6_PEK_RELEASED?TERM_FG_GREEN:TERM_RESET, TERM_RESET);
    printf("%stimer timeout%s\n",IRQ_ENABLE_5_BIT7_TIMER_TIMEOUT?TERM_FG_GREEN:TERM_RESET, TERM_RESET);

}


bool AXP202Driver::GetData(char (&data)[12]) {
    return false;
}

bool AXP202Driver::SaveData(char *data[12]) {
    return false;
}

bool AXP202Driver::ReadStatus() {
    // talk with AXP202
    lunokiot_i2c_channel_descriptor_t myDescriptor = {};
    bool works = i2cHandler->GetSession(frequency, sda, scl, myDescriptor);
    if ( true != works ) {
        debug_printferror("Unable to start i2c session");
        return true; // try again next time
    }
    if ( pdTRUE != xSemaphoreTake(statusMutex, portMAX_DELAY) ) {
        debug_printferror("Unable to get status mutex");
        return true;
    }
    // get status1
    uint8_t status1 = 0x0;
    works = i2cHandler->GetChar(myDescriptor, address, I2C_REGISTER::IRQ_STATUS_1, status1);
    if ( works ) {
        status1 &= 0b01111111; // remove reserved bit in this register
        if ( status1 != IRQ_STATUS_CACHE_CURRENT[0] ) {
            //debug_printf("IRQ_STATUS_1: 0x%x", status1);
            IRQ_STATUS_CACHE_LAST[0] = IRQ_STATUS_CACHE_CURRENT[0]; // backup the last for irq reconeissance
            IRQ_STATUS_CACHE_CURRENT[0] = status1; // update cache
        }
    }
    // get status2
    uint8_t status2 = 0x0;
    works = i2cHandler->GetChar(myDescriptor, address, I2C_REGISTER::IRQ_STATUS_2, status2);
    if ( works ) {
        if ( status2 != IRQ_STATUS_CACHE_CURRENT[1] ) {
            //debug_printf("IRQ_STATUS_2: 0x%x", status2);
            IRQ_STATUS_CACHE_LAST[1] = IRQ_STATUS_CACHE_CURRENT[1]; // backup the last for irq reconeissance
            IRQ_STATUS_CACHE_CURRENT[1] = status2; // update cache
            
        }
    }
    // get status3
    uint8_t status3 = 0x0;
    works = i2cHandler->GetChar(myDescriptor, address, I2C_REGISTER::IRQ_STATUS_3, status3);
    if ( works ) {
        status3 &= 0b11111011; // remove reserved bit in this register
        if ( status3 != IRQ_STATUS_CACHE_CURRENT[2] ) {
            //debug_printf("IRQ_STATUS_3: 0x%x", status3);
            IRQ_STATUS_CACHE_LAST[2] = IRQ_STATUS_CACHE_CURRENT[2]; // backup the last for irq reconeissance
            IRQ_STATUS_CACHE_CURRENT[2] = status3; // update cache
        }
    }
    // get status4
    uint8_t status4 = 0x0;
    works = i2cHandler->GetChar(myDescriptor, address, I2C_REGISTER::IRQ_STATUS_4, status4);
    if ( works ) {
        if ( status4 != IRQ_STATUS_CACHE_CURRENT[3] ) {
            //debug_printf("IRQ_STATUS_4: 0x%x", status4);
            IRQ_STATUS_CACHE_LAST[3] = IRQ_STATUS_CACHE_CURRENT[3]; // backup the last for irq reconeissance
            IRQ_STATUS_CACHE_CURRENT[3] = status4; // update cache
        }
    }
    // get status5
    uint8_t status5 = 0x0;
    works = i2cHandler->GetChar(myDescriptor, address, I2C_REGISTER::IRQ_STATUS_5, status5);
    if ( works ) {
        if ( status5 != IRQ_STATUS_CACHE_CURRENT[4] ) {
            //debug_printf("IRQ_STATUS_5: 0x%x", status5);
            IRQ_STATUS_CACHE_LAST[4] = IRQ_STATUS_CACHE_CURRENT[4]; // backup the last for irq reconeissance
            IRQ_STATUS_CACHE_CURRENT[4] = status5; // update cache
        }
    }
    /*
    debug_printf("=================> READSTATUS:");
    this->DescribeStatus();
    debug_printf("=================> LAST:");
    this->DescribeStatus(IRQ_STATUS_CACHE_LAST);
    */

    xSemaphoreGive(statusMutex);
    // free i2c
    i2cHandler->FreeSession(myDescriptor);
    return true;
}
bool AXP202Driver::StatusChangeActions() {
    debug_printf("\nAnalyse the AXP202 registers to get the current interrupt state");
    bool anyChange = false;
    if ( pdTRUE != xSemaphoreTake(statusMutex, portMAX_DELAY) ) {
        debug_printferror("Unable to get status mutex");
        return false;
    }
    // https://en.wikipedia.org/wiki/Bitwise_operations_in_C
    // get diff from status registers
    //uint8_t changesSTEP[5];
    uint8_t changes[5];
    for ( size_t offset = 0; offset<sizeof(IRQ_STATUS_CACHE_CURRENT);offset++) {
        //changesSTEP[offset] = (IRQ_STATUS_CACHE_CURRENT[offset] ^ IRQ_STATUS_CACHE_LAST[offset]);
        //changes[offset] =  (IRQ_STATUS_CACHE_CURRENT[offset] & changesSTEP[offset]);
        changes[offset] =  IRQ_STATUS_CACHE_CURRENT[offset];
    };

    //debug_printf("Events");
    this->DescribeStatus(IRQ_STATUS_CACHE_CURRENT);
    
    xSemaphoreGive(statusMutex);
    /*
    debug_printf("=================> IRQ_STATUS_CACHE_LAST:");
    this->DescribeStatus(IRQ_STATUS_CACHE_LAST);
    debug_printf("=================> STEP:");
    this->DescribeStatus(changesSTEP);*/
    //debug_printf("Calculated changed flags:");
    //this->DescribeStatus(changes);
    
    //IRQ_STATUS_1_CACHE
    //bool IRQ_ENABLE_1_BIT0_RESERVED = false;
    bool IRQ_ENABLE_1_BIT1_VBUS_LOW = changes[0] >> 1;
    bool IRQ_ENABLE_1_BIT2_VBUS_REMOVED = changes[0] >> 2;
    bool IRQ_ENABLE_1_BIT3_VBUS_CONNECTED = changes[0] >> 3;
    bool IRQ_ENABLE_1_BIT4_VBUS_OVERVOLTAGE = changes[0] >> 4;
    bool IRQ_ENABLE_1_BIT5_ACIN_REMOVED = changes[0] >> 5;
    bool IRQ_ENABLE_1_BIT6_ACIN_CONNECTED = changes[0] >> 6;
    bool IRQ_ENABLE_1_BIT7_ACIN_OVERVOLTAGE = changes[0] >> 7;
    //IRQ_STATUS_2_CACHE
    bool IRQ_ENABLE_2_BIT0_BATT_LOW_TEMP = changes[1];
    bool IRQ_ENABLE_2_BIT1_BATT_OVERHEAT = changes[1] >> 1;
    bool IRQ_ENABLE_2_BIT2_BATT_CHARGED = changes[1] >> 2;
    bool IRQ_ENABLE_2_BIT3_BATT_CHARGING = changes[1] >> 3;
    bool IRQ_ENABLE_2_BIT4_BATT_EXIT_ACTIVATE =  changes[1] >> 4;
    bool IRQ_ENABLE_2_BIT5_BATT_ACTIVATE =  changes[1] >> 5;
    bool IRQ_ENABLE_2_BIT6_BATT_REMOVED =  changes[1] >> 6;
    bool IRQ_ENABLE_2_BIT6_BATT_CONNECTED =  changes[1] >> 7;
    //IRQ_STATUS_3_CACHE
    bool IRQ_ENABLE_3_BIT0_PEK_LONG = changes[2];
    bool IRQ_ENABLE_3_BIT1_PEK_SHORT = changes[2] >> 1;
    bool IRQ_ENABLE_3_BIT2_LDO3_UNDERVOLTAGE =  changes[2] >> 2;
    bool IRQ_ENABLE_3_BIT3_DC_DC3_UNDERVOLTAGE =  changes[2] >> 3;
    bool IRQ_ENABLE_3_BIT4_DC_DC2_UNDERVOLTAGE =  changes[2] >> 4;
    //bool IRQ_ENABLE_3_BIT5_RESERVED = false;
    bool IRQ_ENABLE_3_BIT6_CHARGE_UNDERVOLTAGE =  changes[2] >> 6;
    bool IRQ_ENABLE_3_BIT7_INTERNAL_OVERHEAT =  changes[2] >> 7;
    //IRQ_STATUS_4_CACHE
    bool IRQ_ENABLE_4_BIT0_AXP_UNDERVOLTAGE_LEVEL2 = changes[3];
    bool IRQ_ENABLE_4_BIT1_AXP_UNDERVOLTAGE_LEVEL1 = changes[3] >> 1;
    bool IRQ_ENABLE_4_BIT2_VBUS_SESSION_END = changes[3] >> 2;
    bool IRQ_ENABLE_4_BIT3_VBUS_SESSION_AB = changes[3] >> 3;
    bool IRQ_ENABLE_4_BIT4_VBUS_INVALID = changes[3] >> 4;
    bool IRQ_ENABLE_4_BIT5_VBUS_VALID = changes[3] >> 5;
    bool IRQ_ENABLE_4_BIT6_N_OE_SHUTDOWN = changes[3] >> 6;
    bool IRQ_ENABLE_4_BIT7_N_OE_STARTUP = changes[3] >> 7;
    //IRQ_STATUS_5_CACHE
    bool IRQ_ENABLE_5_BIT0_GPIO0_INPUT = changes[4];
    bool IRQ_ENABLE_5_BIT1_GPIO1_INPUT = changes[4] >>1;
    bool IRQ_ENABLE_5_BIT2_GPIO2_INPUT = changes[4] >>2;
    bool IRQ_ENABLE_5_BIT3_GPIO3_INPUT = changes[4] >>3;
    //bool IRQ_ENABLE_5_BIT4_RESERVED = false;
    bool IRQ_ENABLE_5_BIT5_PEK_PRESS =  changes[4] >>5;
    bool IRQ_ENABLE_5_BIT6_PEK_RELEASED =  changes[4] >>6;
    bool IRQ_ENABLE_5_BIT7_TIMER_TIMEOUT =  changes[4] >>7;

    // check PEK button
    debug_printf("PEK button test part");
    if ( ( IRQ_ENABLE_5_BIT6_PEK_RELEASED ) || ( IRQ_ENABLE_5_BIT5_PEK_PRESS ) ) {
        debug_printf(" * PEK Button state: %s", IRQ_ENABLE_5_BIT6_PEK_RELEASED?"released":"pressed");
        anyChange = true;
    }
    if ( ( IRQ_ENABLE_3_BIT0_PEK_LONG ) || (IRQ_ENABLE_3_BIT1_PEK_SHORT) ) {
        debug_printf(" * PEK Button event: %s press", IRQ_ENABLE_3_BIT1_PEK_SHORT?"short":"long");
        anyChange = true;
    }
    // check timer timeout
    debug_printf("Timer timeout test part");
    if ( IRQ_ENABLE_5_BIT7_TIMER_TIMEOUT ) {
        debug_printf(" * Get Timer timeout");
        lunokiot_i2c_channel_descriptor_t i2cDescriptor = {};
        bool works = i2cHandler->GetSession(frequency, sda, scl, i2cDescriptor);
        if ( works ) {
            i2cHandler->SetChar(i2cDescriptor, address, I2C_REGISTER::TIMER_CONTROL, 0b00000001); // disable timer
            i2cHandler->FreeSession(i2cDescriptor);
            debug_printf("   * Disabled timer (no more timeouts)");
            anyChange = true;
        }
    }
    debug_printf("\nAXP202 registers interpreted, end of interrupt handling");


    debug_printf("@DEBUG test zone");
    // action to do when LONGPRESS
    //if ( ( IRQ_ENABLE_3_BIT0_PEK_LONG ) && (IRQ_ENABLE_3_BIT1_PEK_SHORT) ) {
    if ( ( IRQ_ENABLE_3_BIT0_PEK_LONG ) && (not IRQ_ENABLE_3_BIT1_PEK_SHORT) ) {
        this->DumpRegisters();
        anyChange = true;
    }
    
    return anyChange;
}
/*
//#define BIT_MASK(x) (1 << x)
AXP202Driver::PEK_BUTTON AXP202Driver::PekButtonState(uint8_t status[5]=IRQ_STATUS_CACHE_CURRENT) {
    return PEK_BUTTON(status[2] & (1 << 1));
}*/
/*
bool AXP20X_Class::isPEKLongtPressIRQ()
{
    return (bool)(_irq[2] & BIT_MASK(0));
}
*/

bool AXP202Driver::PoolRegisters() {
    // talk with AXP202
    lunokiot_i2c_channel_descriptor_t myDescriptor = {};
    bool works = i2cHandler->GetSession(frequency, sda, scl, myDescriptor);
    if ( true != works ) {
        debug_printferror("Unable to start i2c session");
        return false; // try again next time
    }
    // get OFF_CONTROL
    uint8_t offControlData = 0x0;
    works = i2cHandler->GetChar(myDescriptor, address, I2C_REGISTER::OFF_CONTROL, offControlData);
    if ( not works ) {
        debug_printferror("Unable to get i2c data");
        return false;
    }
    //debug_printf("off control register got 0x%x", offControlData);
    
    /*
    uint8_t delayShutdown = offControlData & 0b00000011;
    // [0~1]=DELAY SHUTDOWN (00=128ms, 01=1s, 10=2s, 11=3s)
    debug_printf("delay shutdown 0x%x", delayShutdown);
    */

    // [2]=output disable timing control (0=disable at same time, 1=contrary to startup timming)

/*
    bool chargeLed = offControlData >> 3;
    debug_printf("Charger led: %s", chargeLed?"in use by charger":"free to use");
*/


// shutdown settings, battery detection and charge led
// bits: 
// [3]=chrgled (0=controled by charging, 1=controlled by next register at [4~5] offset)
// [4~5]=chrgled pin function (00=high resistance, 01=25% 1Hz flicker, 10=25% 4Hz flicker, 11=low level output)
// [6]=batt monitor (0=no,1=yes)
// [7]=shutdown

    // free i2c
    i2cHandler->FreeSession(myDescriptor);
    return true;
}


// AXP202 management via timer (see ->period value)
bool AXP202Driver::Loop() {
    
    // have pending to read interrupts?
    if ( true == axp202Interrupt ) { 
        debug_printf("AXP202 Interrupt received");
        this->ReadStatus(); // get status data
        this->StatusChangeActions(); // take decisions with the changes
        this->Clearbits(); // ack received data from AXP
        axp202Interrupt = false; // ack interrupt 
    }

    // is time to refresh all registers?
    static TickType_t nextPool = 0; // force first refresh
    if ( nextPool <  xTaskGetTickCount() ) {
        //debug_printf("Obtaining AXP202 registers...");
        this->PoolRegisters();
        nextPool = xTaskGetTickCount() + APX202POOL_TIME_MS;
    }

   return true;
}
