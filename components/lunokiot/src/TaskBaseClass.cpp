#include "LunokIoT.hpp"
#include "base/TaskTemplate.hpp"

#include <iostream>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

using namespace LunokIoT;


#define CONFIG_LUNOKIOT_DEBUG_TASK // @TODO debugging


#ifdef CONFIG_LUNOKIOT_DEBUG_TASK
#define debug_write(...) printf(__VA_ARGS__);
#define debug_header() debug_write("%08u %s:%d %s() ",xTaskGetTickCount(),__FILE__, __LINE__, __func__);
#define debug_printf(...) { \
                debug_header(); \
                debug_write(__VA_ARGS__); \
                fflush(stdout); \
}
#else
#define debug_write(...)
#define debug_printf(...)
#endif //CONFIG_LUNOKIOT_DEBUG_TASK


// task creation id
::SemaphoreHandle_t _TaskBaseClassCounterLock = xSemaphoreCreateMutex();
static size_t _TaskBaseClassCounter = 0; // task creation counter

TaskBaseClass::TaskBaseClass(const char * name, unsigned long period) 
                                : name(name), period(period) {
    // mutex for counter
    xSemaphoreTake(_TaskBaseClassCounterLock, portMAX_DELAY);
    _TaskBaseClassCounter++;
    id = _TaskBaseClassCounter;
    xSemaphoreGive(_TaskBaseClassCounterLock);

    this->Lock = xSemaphoreCreateMutex();
    /*
    if ( -1 == this->_period ) {
        printf("%p %s Task don't have loop\n", this, this->name);
        return;
    }
    */
    //printf("%p %s Task callback every: %lums \n", this, this->name, this->_period);
    xTaskCreate(
        &TaskBaseClass::Callback,                    // Function that should be called
        name,                         // Name of the task (for debugging)
        2000,                               // Stack size (bytes) @TODO must be set to configMINIMAL_STACK_SIZE
        this,                               // Parameter to pass
        tskIDLE_PRIORITY,                   // Task priority
        &taskHandle                        // Task handle
    );

    debug_printf("new\n");
};



TaskBaseClass::~TaskBaseClass() {
    debug_printf("delete\n");
}
