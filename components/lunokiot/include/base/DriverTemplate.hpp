#ifndef ___LUNOKIOT___DRIVER___BASE___
#define ___LUNOKIOT___DRIVER___BASE___

#include "../LunokIoT.hpp"
#include "../Task.hpp"
#include "DeviceTemplate.hpp"

//@TODO reflect this on Kconfig
#ifndef CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME
#define CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME -1 /* task disable */
#endif //CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME

using namespace LunokIoT;

class DriverBaseClass : public Task {
    public:
        DriverBaseClass(const char *name=(const char*)"NOTSET_DEVICE", unsigned long loopPeriod=CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME) 
                            : Task(name, loopPeriod) {
            printf("%p %s Setup (loop: %lums)\n", this, name, _period);
        }
        bool Loop() { return true; };
};

class DriverTemplate : public Task {
    public:
        DriverTemplate(const char *deviceName=(const char*)"Driver template", unsigned long period=50) : Task(deviceName, period) {
            
        }
        virtual bool Loop() = 0;
};

#endif // ___LUNOKIOT___DRIVER___BASE___
