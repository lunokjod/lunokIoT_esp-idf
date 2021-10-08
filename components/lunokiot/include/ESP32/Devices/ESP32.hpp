#ifndef __LUNOKIOT__ESP32_DEVICE___
#define __LUNOKIOT__ESP32_DEVICE___


#include "LunokIoT.hpp"
#include "../Device.hpp"
#include "../Drivers/NVS.hpp"
#include "../Drivers/Console.hpp"
#include "../Drivers/WiFi.hpp"
#include "../Services/NTPService.hpp"
#include "../Drivers/I2C.hpp"

namespace LunokIoT {
//@TODO https://stackoverflow.com/questions/11711920/how-to-implement-multithread-safe-singleton-in-c11-without-using-mutex
    class ESP32Device : public Device {
        public:
            static int Restart(int argc, char **argv);
            static int FreeMem(int argc, char **argv);
            static int Heap(int argc, char **argv);
            static int Tasks(int argc, char **argv);
            static int Scheduler(int argc, char **argv);
            static int Info(int argc, char **argv);
            static int LightSleep(int argc, char **argv);
            static int DeepSleep(int argc, char **argv);
            ESP32Device();
            bool Loop();
            void RegisterConsoleCommands(void);
            NVSDriver * nvs = nullptr;
            ConsoleDriver * console = nullptr;
            WiFiDriver * wifi = nullptr;
            I2CDriver  * i2c = nullptr;
            NTPService * ntp = nullptr;
    };

}

#endif // __LUNOKIOT__ESP32_DEVICE___
