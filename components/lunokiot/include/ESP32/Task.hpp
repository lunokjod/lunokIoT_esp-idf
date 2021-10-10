#ifndef ___LUNOKIOT__ESP32_TASK____
#define ___LUNOKIOT__ESP32_TASK____

#include "LunokIoT.hpp"
#include "Debug.hpp"
#include "base/TaskTemplate.hpp"

#include <limits.h>
#include <climits>
#include <cstdlib>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_system.h>


namespace LunokIoT {

    class Task : public TaskTemplate {
        public:
            ::SemaphoreHandle_t Lock;
            char * name =nullptr;
            static size_t CreatedTasks;
            Task(const char* name, unsigned long period);
            virtual bool Loop() { // return false if want to end the task
                printf("%p %s id: %u Loop\n", this, this->name, this->_id);
                return true;
            };
            void Suspend();
            void Resume();
            [[noreturn]] static void Callback(void * payload) {
                LunokIoT::Task *instance = static_cast<LunokIoT::Task*>(payload);
                TickType_t xLastWakeTime = xTaskGetTickCount();
                while(true) {
                    if ( -1 == instance->_period) {
                        instance->Suspend();
                        //vTaskDelay( LONG_MAX-1 / portTICK_PERIOD_MS);
                        continue;
                    }
                    //uint32_t b4 = xTaskGetTickCount();
                    //printf("@DEBUG %p before: %06d '%s'\n", instance,  b4 / portTICK_PERIOD_MS, instance->name);
                    vTaskDelayUntil( &xLastWakeTime, instance->_period * portTICK_PERIOD_MS );
                    //uint32_t ar = xTaskGetTickCount();
                    //printf("@DEBUG %p  after: %06dms '%s'\n", instance, (ar-b4) / portTICK_PERIOD_MS, instance->name);
                    
                    //vTaskDelay(); // maybe other type of task ;-)
                    //DebugPrintf("callback\n");
                    xSemaphoreTake(instance->Lock, portMAX_DELAY);
                    bool done = instance->Loop();
                    xSemaphoreGive(instance->Lock);
                    if ( not done ) {
                        printf("%p %s task stop\n", instance, instance->name);
                        vTaskDelete(NULL);                        
                    }
                }
            };
            ~Task() {
                if(_taskHandle != NULL) {
                    vTaskDelete(_taskHandle);
                }
                if ( nullptr !=  this->name ) {
                    free(this->name);
                }
                DebugOPrintf("delete\n");
            };
            TaskHandle_t _taskHandle = NULL;
            size_t _id = 0;
    };

}
#endif // ___LUNOKIOT__ESP32_TASK____
