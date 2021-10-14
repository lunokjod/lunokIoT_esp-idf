#ifndef ____LUNOKIOT___DRIVER_BUTTON____
#define ____LUNOKIOT___DRIVER_BUTTON____

#include "LunokIoT.hpp"
#include "../Driver.hpp"
#include <hal/gpio_types.h>

namespace LunokIoT {

    class ButtonDriver : public Driver {
        public:
            ButtonDriver(gpio_num_t gpio);
            bool Loop();
            gpio_num_t gpio;
            int lastVal = 1;
    };

}

#endif // ____LUNOKIOT___DRIVER_BUTTON____
