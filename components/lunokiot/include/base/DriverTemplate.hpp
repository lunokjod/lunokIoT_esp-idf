#ifndef ___LUNOKIOT___DRIVER___BASE___
#define ___LUNOKIOT___DRIVER___BASE___

#include "../LunokIoT.hpp"
#include "TaskTemplate.hpp"
//#include "../Task.hpp"

//@TODO reflect this on Kconfig
#ifndef CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME
#define CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME -1 /* task suspended */
#endif //CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME

using namespace LunokIoT;

class DriverBaseClass : public TaskBaseClass {
    public:
        DriverBaseClass(const char *name=(const char*)"NOTSET_DEVICE", unsigned long period=CONFIG_DEFAULT_LUNOKIOT_DRIVER_LOOP_TIME)
                            : TaskBaseClass(name, period, 6000) {
            //debug_printf("Setup (loop: %lums)", period);
        }
        bool Loop() {  // implement for testing purposes
            //debug_printf("DriverBaseClass");
            return true;
        }
};

class DriverTemplate : public Task {
    public:
        DriverTemplate(const char *deviceName=(const char*)"Driver template", unsigned long period=50) : Task(deviceName, period) {

        }
        virtual bool Loop() = 0;
};

#endif // ___LUNOKIOT___DRIVER___BASE___
