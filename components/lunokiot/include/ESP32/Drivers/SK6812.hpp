#ifndef ____LUNOKIOT___DRIVER_SK6812____
#define ____LUNOKIOT___DRIVER_SK6812____

#include "LunokIoT.hpp"
#include "../Driver.hpp"
#include <hal/gpio_types.h>

namespace LunokIoT {

    union RGBColor {
        uint32_t raw;
        struct  {
            uint8_t g;
            uint8_t r;
            uint8_t b;
            uint8_t a; // unused upper byte.
    	};
    } __packed;

    class SK6812Driver : public Driver {
        public:
            SK6812Driver(gpio_num_t gpio);
            bool Loop();
            void SetLedColor(uint32_t color);
            gpio_num_t gpio;
            RGBColor currentLedColor;
            static int _SetColorCmd(int argc, char **argv);
    };

}

#endif // ____LUNOKIOT___DRIVER_SK6812____
