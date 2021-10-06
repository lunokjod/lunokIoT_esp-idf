#ifndef __LUNOKIOT__M5ATOMLITE___
#define __LUNOKIOT__M5ATOMLITE___


#include "LunokIoT.hpp"
#include "../Device.hpp"
#include "../Driver/NVS.hpp"
#include "../Driver/Console.hpp"
#include "../Driver/WiFi.hpp"
#include "../Service/NTPService.hpp"
#include "../Driver/I2C.hpp"

namespace LunokIoT {
//@TODO https://stackoverflow.com/questions/11711920/how-to-implement-multithread-safe-singleton-in-c11-without-using-mutex
    class M5AtomLiteDevice : public Device {
        public:
            static int Restart(int argc, char **argv);
            static int FreeMem(int argc, char **argv);
            static int Heap(int argc, char **argv);
            static int Tasks(int argc, char **argv);
            static int Scheduler(int argc, char **argv);
            static int Info(int argc, char **argv);
            static int LightSleep(int argc, char **argv);
            static int DeepSleep(int argc, char **argv);
            M5AtomLiteDevice();
            bool Loop();
            void RegisterConsoleCommands(void);
            NVSDriver * nvs = nullptr;
            ConsoleDriver * console = nullptr;
            WiFiDriver * wifi = nullptr;
            I2CDriver  * i2c = nullptr;
            NTPService * ntp = nullptr;
    };

}

#endif // __LUNOKIOT__M5ATOMLITE___
