#ifndef ____LUNOKIOT___DRIVER_I2C____
#define ____LUNOKIOT___DRIVER_I2C____

#include "LunokIoT.hpp"
#include "../Driver.hpp"

namespace LunokIoT {
    class I2CDriver : public Driver {
        public:
            I2CDriver();
            bool Loop();
    };

}

#endif // ____LUNOKIOT___DRIVER_I2C____
