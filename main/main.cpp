#include <iostream>
#include <LunokIoT.hpp>
#include <freertos/task.h>

extern "C" void app_main(void) {
    std::cout << "app_main starting" << std::endl;
    LunokIoT::Begin();
    while(true) {
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        fflush(stdout);
    }
    std::cout << "app_main done" << std::endl;
}
