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
    printf("%p %s Setup\n", this, this->name);
    gpio_set_direction(this->gpio, GPIO_MODE_INPUT);
}
bool ButtonDriver::Loop() {
    int val = gpio_get_level(this->gpio);
    if ( val != lastVal ) {
        printf("%p %s(%d) ", this, this->name, gpio);
        printf("%s ", val?"Released":"Pressed" );
        lastVal = val;
        TickType_t thisEvent = xTaskGetTickCount();        
        if ( ButtonStatus::Pressed == val ) {
            lastEvent = thisEvent;
        } else if ( ButtonStatus::Released == val ) {
            TickType_t diffTime = thisEvent-lastEvent;
            if ( diffTime > 0 ) {
                printf("%dms ", diffTime);
                if ( diffTime > 400 ) {
                    printf(" long press");
                } else {
                    printf(" clicked");
                }
            }
            lastEvent = thisEvent;
        }
        printf("\n");
    }
    return true;
}
