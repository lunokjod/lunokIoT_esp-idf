#ifndef ___LUNOKIOT__ESP32_DEVICE___
#define ___LUNOKIOT__ESP32_DEVICE___

#include "LunokIoT.hpp"
#include "Debug.hpp"
#include "base/DeviceTemplate.hpp"

namespace LunokIoT {

    class Device : public DeviceTemplate {
        public:
            Device(const char* name=(const char*)"Null Device");
            bool Loop();
    };

}

#endif // ___LUNOKIOT__ESP32_DEVICE___
