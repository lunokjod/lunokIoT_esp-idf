#ifndef ___LUNOKIOT___DEVICE___BASE___
#define ___LUNOKIOT___DEVICE___BASE___

#include "../LunokIoT.hpp"
#include "../Task.hpp"
//#include "Debug.hpp"

using namespace LunokIoT;

class DeviceTemplate : public Task { //@TODO must be renamed to DeviceBaseClass
    public:
        DeviceTemplate(const char *deviceName=(const char*)"Device template") : Task(deviceName, 50) {
            
        }
        virtual bool Loop() = 0;
};

#endif // ___LUNOKIOT___DEVICE___BASE___
