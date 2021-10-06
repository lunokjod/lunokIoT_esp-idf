#ifndef ___LUNOKIOT___SERVICE___BASE___
#define ___LUNOKIOT___SERVICE___BASE___

#include "../LunokIoT.hpp"
#include "../Task.hpp"
using namespace LunokIoT;

class ServiceTemplate : public Task {
    public:
        ServiceTemplate(const char *deviceName=(const char*)"Service template") : Task(deviceName, 60000) {
            
        }
        virtual bool Loop() = 0;
};

#endif // ___LUNOKIOT___SERVICE___BASE___
