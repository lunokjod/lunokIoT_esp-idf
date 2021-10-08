#ifndef __LUNOKIOT__M5ATOMLITE___
#define __LUNOKIOT__M5ATOMLITE___


#include "LunokIoT.hpp"
#include "../Device.hpp"
#include "ESP32.hpp"

namespace LunokIoT {
//@TODO https://stackoverflow.com/questions/11711920/how-to-implement-multithread-safe-singleton-in-c11-without-using-mutex
    class M5AtomLiteDevice : public ESP32Device {
        public:
            M5AtomLiteDevice();
            bool Loop();
    };

}

#endif // __LUNOKIOT__M5ATOMLITE___
