#ifndef ____LUNOKIOT___DRIVER_BUTTON____
#define ____LUNOKIOT___DRIVER_BUTTON____

#include "LunokIoT.hpp"
#include "../Driver.hpp"
#include <hal/gpio_types.h>
#include <freertos/task.h>

namespace LunokIoT {

    class ButtonDriver : public Driver {
        public:
            ButtonDriver(gpio_num_t gpio);
            bool Loop();
            gpio_num_t gpio;
            int lastVal = 1;
            TickType_t lastEvent = 0;
    };
    
    /* @TODO i2c button
    class I2CButtonDriver: public ButtonDriver {
            I2CButtonDriver();
            bool Loop();
    };*/

}

#endif // ____LUNOKIOT___DRIVER_BUTTON____
