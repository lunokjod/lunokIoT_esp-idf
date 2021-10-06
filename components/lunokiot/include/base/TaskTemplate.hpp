#ifndef ___LUNOKIOT___TASK___BASE___
#define ___LUNOKIOT___TASK___BASE___

#include "../LunokIoT.hpp"
//#include "../Task.hpp"

using namespace LunokIoT;

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
