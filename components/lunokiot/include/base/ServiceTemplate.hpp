#ifndef ___LUNOKIOT___SERVICE___BASE___
#define ___LUNOKIOT___SERVICE___BASE___

#include "../LunokIoT.hpp"
//#include "../Task.hpp"
#include "TaskTemplate.hpp"
using namespace LunokIoT;

class ServiceTemplate : public TaskBaseClass {
    public:
        ServiceTemplate(const char *deviceName=(const char*)"Service template", unsigned long period=60000, uint32_t stackSize= configMINIMAL_STACK_SIZE) : 
                                    TaskBaseClass(deviceName, period, stackSize) {
                                        
                                    }
        virtual bool Loop() = 0;
};

#endif // ___LUNOKIOT___SERVICE___BASE___
