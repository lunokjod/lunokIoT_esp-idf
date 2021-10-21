#ifndef ___LUNOKIOT__ESP32_DRIVER___
#define ___LUNOKIOT__ESP32_DRIVER___

#include "LunokIoT.hpp"
#include "base/DriverTemplate.hpp"

namespace LunokIoT {
    class Driver : public DriverTemplate {
        public:
            Driver(const char* name=(const char*)"Null Driver", unsigned long period=(unsigned long)-1);
            bool Loop();
    };

}

#endif // ___LUNOKIOT__ESP32_DRIVER___
