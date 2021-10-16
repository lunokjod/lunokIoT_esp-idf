#ifndef __LUNOKIOT__M5STICKCPLUS__
#define __LUNOKIOT__M5STICKCPLUS__


#include "LunokIoT.hpp"
#include "../Device.hpp"
#include "ESP32.hpp"
#include "../Drivers/Button.hpp"
#include "../Drivers/AXP192.hpp"

namespace LunokIoT {
//@TODO https://stackoverflow.com/questions/11711920/how-to-implement-multithread-safe-singleton-in-c11-without-using-mutex
    class M5StickCPlusDevice : public ESP32Device {
        public:
            M5StickCPlusDevice();
            AXP192Driver * axp192 = nullptr;
            ButtonDriver * button0 = nullptr;
            ButtonDriver * button1 = nullptr;
    };

}

#endif // __LUNOKIOT__M5STICKCPLUS__
