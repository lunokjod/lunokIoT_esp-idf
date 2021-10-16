#ifndef ____LUNOKIOT___DRIVER_I2C____
#define ____LUNOKIOT___DRIVER_I2C____

#include "LunokIoT.hpp"
#include "../Driver.hpp"
#include "driver/i2c.h"

namespace LunokIoT {
    class I2CDriver : public Driver {
        public:
            I2CDriver();
            bool Loop();
    };

}

extern "C" {
    esp_err_t i2c_get_port(int port, i2c_port_t *i2c_port);

    //i2c_port_t i2c_port; // = I2C_NUM_0;
}
#endif // ____LUNOKIOT___DRIVER_I2C____
