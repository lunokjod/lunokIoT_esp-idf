#ifndef ___LUNOKIOT___DRIVER___BASE___
#define ___LUNOKIOT___DRIVER___BASE___

#include "../LunokIoT.hpp"
#include "../Task.hpp"

using namespace LunokIoT;

class DriverTemplate : public Task {
    public:
        DriverTemplate(const char *deviceName=(const char*)"Driver template", unsigned long period=50) : Task(deviceName, period) {
            
        }
        virtual bool Loop() = 0;
};

#endif // ___LUNOKIOT___DRIVER___BASE___
