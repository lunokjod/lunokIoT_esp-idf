#ifndef ____LUNOKIOT___DRIVER_LEDC____
#define ____LUNOKIOT___DRIVER_LEDC____

#include "LunokIoT.hpp"
#include "base/DriverTemplate.hpp"
#include <hal/gpio_types.h>

namespace LunokIoT {


    class LEDDriver : public DriverBaseClass {
        public:

            LEDDriver();
            bool Loop();
    };

}

#endif // ____LUNOKIOT___DRIVER_LEDC____
