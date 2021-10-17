#ifndef ____LUNOKIOT___DRIVER_I2C____
#define ____LUNOKIOT___DRIVER_I2C____

#include "LunokIoT.hpp"
#include "../Driver.hpp"
#include "driver/i2c.h"

namespace LunokIoT {

#define I2C_DEFAULT_PORT I2C_NUM_1
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */
#define I2C_MASTER_TIMEOUT_MS       500

    class I2CDriver : public Driver {
        public:
            I2CDriver();
            bool Loop();
    };

}

extern "C" {
    esp_err_t i2c_get_port(int port, i2c_port_t *i2c_port);
}
#endif // ____LUNOKIOT___DRIVER_I2C____
