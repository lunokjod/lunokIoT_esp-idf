#ifndef ____LUNOKIOT___DRIVER_I2C____
#define ____LUNOKIOT___DRIVER_I2C____

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/i2c.h>

#include "LunokIoT.hpp"
#include "../Driver.hpp"


typedef struct {
    SemaphoreHandle_t useLock; // must be set on GetSession and freed on FreeSession
    i2c_port_t port; // redundant, is the offset of struct (see class member "channels")
    uint32_t frequency;
    gpio_num_t sda;
    gpio_num_t scl;
    TickType_t lastUsed;
    size_t referenceCounter=0;

} lunokiot_i2c_channel_descriptor_t;

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
#define I2C_MASTER_TIMEOUT_MS       300

    class I2CDriver : public Driver {
        private:
            SemaphoreHandle_t _mutexLock = xSemaphoreCreateMutex();

        public:
            // call this to get i2c
            bool GetSession(uint32_t i2cfrequency, 
                        gpio_num_t i2csdagpio, gpio_num_t i2csclgpio, lunokiot_i2c_channel_descriptor_t &descriptor);
            // cal this when you i2c comms has ended
            bool FreeSession(lunokiot_i2c_channel_descriptor_t &descriptor);
            bool GetChar(lunokiot_i2c_channel_descriptor_t &descriptor, uint8_t address, const uint8_t i2cregister, uint8_t &value);
            bool SetChar(lunokiot_i2c_channel_descriptor_t &descriptor, uint8_t address, const uint8_t i2cregister, const uint8_t value);
            void CleanupSessions(); // some kind of garbage collector x'D

            /*
            [[deprecated("Please use GetSession instead")]]
            bool GetI2CSession(i2c_port_t i2cport, uint32_t i2cfrequency, 
                        gpio_num_t i2csdagpio, gpio_num_t i2csclgpio,
                        uint8_t i2caddress);*/
            //[[deprecated("Please use FreeSession instead")]]
            //bool FreeI2CSession(i2c_port_t i2cport);
            //[[deprecated("Please use SetChar instead")]]
            //bool SetI2CChar(i2c_port_t i2cport, uint8_t address, const uint8_t i2cregister, const uint8_t value);
            //[[deprecated("Please use GetChar instead")]]
            //bool GetI2CChar(i2c_port_t i2cport, uint8_t address, const uint8_t i2cregister, uint8_t &value);
        public:
            I2CDriver();
            bool Loop();
    };

}

extern "C" {
    esp_err_t i2c_get_port(int port, i2c_port_t *i2c_port);
}
#endif // ____LUNOKIOT___DRIVER_I2C____
