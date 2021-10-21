#include <iostream>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "LunokIoT.hpp"



extern "C" void app_main(void) {
    debug_printf("app_main starting");
    LunokIoT::Begin();
    // app code must be implemented here
    while(true) {
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        //fflush(stdout);
    }
    debug_printf("app_main done (If you see this, must be code problem)");
}
