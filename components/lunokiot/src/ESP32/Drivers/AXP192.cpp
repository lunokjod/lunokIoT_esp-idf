#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/AXP192.hpp"
#include <hal/gpio_types.h>
#include "driver/i2c.h"
#include "ESP32/Drivers/I2C.hpp"

using namespace LunokIoT;

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

extern "C" i2c_port_t i2c_port;
//extern "C" esp_err_t i2c_get_port(int port, i2c_port_t *i2c_port);

AXP192Driver::AXP192Driver(gpio_num_t sdagpio, gpio_num_t sclgpio): 
                                       Driver((const char*)"(-) AXP192", (unsigned long)100), 
                                        sda(sdagpio), scl(sclgpio) {
    printf("%p %s Setup\n", this, this->name);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
    
    // Step 1, configure:
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (int)sda,         // select GPIO specific to your project
        .scl_io_num = (int)scl,         // select GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    conf.master.clk_speed = 100000;  // select frequency specific to your project

    esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
    printf("CHECK0: %s\n", esp_err_to_name(ret));

    // Step 2, install driver:
    ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    printf("CHECK1: %s\n", esp_err_to_name(ret));
    ret = i2c_get_port(I2C_NUM_0, &i2c_port);
    printf("CHECK2: %s\n", esp_err_to_name(ret));


    // Step 3, master/slave comms
    // Step 4, interrupt handling
    // Step 5, custom config
    // Step 6, error handling
    // Steo 7, deinstall the driver

    uint8_t address = CONTROL::POWER_BUTTON;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ret = i2c_master_start(cmd);
    printf("CHECK3: %s\n", esp_err_to_name(ret));
    ret = i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
    printf("CHECK4: %s\n", esp_err_to_name(ret));
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 50 / portTICK_RATE_MS);
    printf("CHECK5: %s\n", esp_err_to_name(ret));
    ret = i2c_master_stop(cmd);
    printf("CHECK6: %s\n", esp_err_to_name(ret));
    i2c_cmd_link_delete(cmd);



    ret = i2c_driver_delete(I2C_NUM_0);
    printf("CHECK7: %s\n", esp_err_to_name(ret));
}
