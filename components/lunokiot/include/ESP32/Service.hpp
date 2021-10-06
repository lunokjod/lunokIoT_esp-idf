#ifndef ___LUNOKIOT__ESP32_SERVICE___
#define ___LUNOKIOT__ESP32_SERVICE___

#include "LunokIoT.hpp"
#include "base/ServiceTemplate.hpp"

namespace LunokIoT {

    class Service : public ServiceTemplate {
        public:
            Service(const char* name=(const char*)"Null Service");
            bool Loop();
    };

}

#endif // ___LUNOKIOT__ESP32_SERVICE___
