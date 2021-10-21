#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/Button.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

enum ButtonStatus {
    Pressed = 0,
    Released = 1
};
/*
static const char *ButtonStatusAsText[] = {
    [ButtonStatus::Pressed]      = "pressed",
    [ButtonStatus::Released]     = "released"
};*/

using namespace LunokIoT;

// https://github.com/UncleRus/esp-idf-lib/tree/master/examples/button/main
ButtonDriver::ButtonDriver(gpio_num_t gpio): Driver((const char*)"(-) Button", (unsigned long)100), gpio(gpio) {
    debug_printf("%p %s Setup (gpio: %d)", this, name, gpio);
    gpio_set_direction(this->gpio, GPIO_MODE_INPUT);
}
bool ButtonDriver::Loop() {
    int val = gpio_get_level(this->gpio);
    //printf("%p %s val: %d old: %d period: %lu\n", this, name, val, lastVal, _period);
    if ( val != lastVal ) {
        printf("%p %s(%d) ", this, this->name, gpio);
        printf("%s ", val?"Released":"Pressed" );
        lastVal = val;
        TickType_t thisEvent = xTaskGetTickCount();        
        if ( ButtonStatus::Pressed == val ) {
            lastEvent = thisEvent;
            this->_period = 3; // button pressed, high resolution poll to get accurated time
        } else if ( ButtonStatus::Released == val ) {
            TickType_t diffTime = thisEvent-lastEvent;
            if ( diffTime > 0 ) {
                printf("%dms ", diffTime);
                if ( diffTime > 499 ) {
                    printf("long press");
                } else {
                    printf("click");
                }
                printf(" event");
            }
            this->_period = 100; // return to normal state
            lastEvent = thisEvent;
        }
        printf("\n");
    }
    return true;
}
