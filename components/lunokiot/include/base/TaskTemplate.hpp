#ifndef ___LUNOKIOT___TASK___BASE___
#define ___LUNOKIOT___TASK___BASE___

#include <iostream>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "../LunokIoT.hpp"


using namespace LunokIoT;


class TaskBaseClass {
    public:
        ::SemaphoreHandle_t Lock;
        TaskBaseClass(const char * name, unsigned long period);
        virtual ~TaskBaseClass();
        virtual bool Loop() { return false; }; // must return true if you want to be called again
        void Suspend() { vTaskSuspend(taskHandle); }
        void Resume() { vTaskResume(taskHandle); }
        [[noreturn]] static void Callback(void * payload) {
            TaskBaseClass *instance = static_cast<TaskBaseClass*>(payload);
            TickType_t xLastWakeTime = xTaskGetTickCount();
            while(true) {
                if ( -1 == instance->period) {
                    instance->Suspend();
                    //vTaskDelay( LONG_MAX-1 / portTICK_PERIOD_MS);
                    continue;
                }
                //uint32_t b4 = xTaskGetTickCount();
                //printf("@DEBUG %p before: %06d '%s'\n", instance,  b4 / portTICK_PERIOD_MS, instance->name);
                vTaskDelayUntil( &xLastWakeTime, instance->period * portTICK_PERIOD_MS );
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
    protected:
        size_t id;
        const char * name;
        unsigned long period;
        TaskHandle_t taskHandle = NULL;
};

class TaskTemplate {
    public:
        TaskTemplate(unsigned long period) : _period(period) {
            //DebugOPrintf("new\n");
        };
        virtual ~TaskTemplate() {
            //DebugOPrintf("delete\n");
        }
        virtual bool Loop() = 0; // implement in child, return false must stop the task
    protected:
        unsigned long _period;
};

#endif // ___LUNOKIOT___TASK___BASE___
