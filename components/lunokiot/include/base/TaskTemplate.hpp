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
        TaskBaseClass(const char * name, unsigned long period);
        virtual ~TaskBaseClass();
        virtual bool Loop() { return false; }; // must return true if you want to be called again
    protected:
        size_t id;
        const char * name;
        unsigned long period;
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
