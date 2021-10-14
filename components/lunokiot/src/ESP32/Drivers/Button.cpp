#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/Button.hpp"

#include <freertos/FreeRTOS.h>
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
        printf("%s\n", val?"Released":"Pressed" );
        lastVal = val;
    }
    return true;
}
