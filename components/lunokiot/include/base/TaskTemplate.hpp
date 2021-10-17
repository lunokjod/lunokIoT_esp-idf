#ifndef ___LUNOKIOT___TASK___BASE___
#define ___LUNOKIOT___TASK___BASE___

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../LunokIoT.hpp"

#define CONFIG_LUNOKIOT_DEBUG_TASK // debugging


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

using namespace LunokIoT;

class TaskBaseClass {
    public:
        TaskBaseClass(const char * const name, unsigned long period) : name(name), period(period) {
            debug_printf("new\n");
        };
        virtual ~TaskBaseClass() {
            debug_printf("delete\n");
        }
        virtual bool Loop() = 0; // implement in child, return false must stop the task
    protected:
        const char * const name;
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
