#include "LunokIoT.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//#include "Debug.hpp"
#include "base/TaskTemplate.hpp"
#include "Task.hpp"

#include <cstring>

using namespace LunokIoT;

size_t Task::CreatedTasks = 0;
void Task::Suspend() {
    vTaskSuspend(this->_taskHandle);
}
void Task::Resume() {
    vTaskResume(this->_taskHandle);
}
Task::Task(const char * name, unsigned long period) : TaskTemplate(period) { 
    this->Lock = xSemaphoreCreateMutex();
    this->_id = Task::CreatedTasks;
    // explicit name copy
    size_t nameLen = strlen(name);
    this->name = (char*)malloc(nameLen+1);
    strcpy(this->name, name);
    Task::CreatedTasks++;
    /*
    if ( -1 == this->_period ) {
        printf("%p %s Task don't have loop\n", this, this->name);
        return;
    }
    */
    //printf("%p %s Task callback every: %lums \n", this, this->name, this->_period);
    xTaskCreate(
        &Task::Callback,                    // Function that should be called
        this->name,                         // Name of the task (for debugging)
        2000,                               // Stack size (bytes) @TODO must be set to configMINIMAL_STACK_SIZE
        this,                               // Parameter to pass
        tskIDLE_PRIORITY,                   // Task priority
        &_taskHandle                        // Task handle
    );
};
