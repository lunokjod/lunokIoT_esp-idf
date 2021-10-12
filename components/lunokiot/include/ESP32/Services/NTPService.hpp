#ifndef ___LUNOKIOT__ESP32_SERVICE___NTP___
#define ___LUNOKIOT__ESP32_SERVICE___NTP___

#include "LunokIoT.hpp"
#include "../Service.hpp"

namespace LunokIoT {

    class NTPService : public Service {
        public:
            NTPService();
            bool Loop();
            static int _DateCmd(int argc, char **argv);
    };

}

#endif // ___LUNOKIOT__ESP32_SERVICE___NTP___
