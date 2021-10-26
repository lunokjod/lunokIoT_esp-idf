#ifndef __LUNOKIOT__M5ATOMLITE___
#define __LUNOKIOT__M5ATOMLITE___


#include "LunokIoT.hpp"
#include "../Device.hpp"
#include "ESP32.hpp"
#include "../Drivers/AXP202.hpp"
#include "../Drivers/Button.hpp"

#define I2C_PORT I2C_NUM_0
// internal
#define I2C0_SCL gpio_num_t(22)
#define I2C0_SDA gpio_num_t(21)

// touch
#define I2C1_SCL gpio_num_t(32)
#define I2C1_SDA gpio_num_t(23)

namespace LunokIoT {
//@TODO https://stackoverflow.com/questions/11711920/how-to-implement-multithread-safe-singleton-in-c11-without-using-mutex
    class LilygoTWatch2020v3Device : public ESP32Device {
        public:
            LilygoTWatch2020v3Device();
            //bool Loop();
            //SK6812Driver * sk6812 = nullptr;
            ButtonDriver * button = nullptr;
            AXP202Driver * axp202 = nullptr;
    };

}

#endif // __LUNOKIOT__M5ATOMLITE___
