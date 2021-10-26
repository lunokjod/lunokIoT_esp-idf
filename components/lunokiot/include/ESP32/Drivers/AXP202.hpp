#ifndef ____LUNOKIOT___DRIVER_AXP192____
#define ____LUNOKIOT___DRIVER_AXP192____

#include "LunokIoT.hpp"
#include "base/DriverTemplate.hpp"
#include <hal/gpio_types.h>
#include <driver/i2c.h>
#include "ESP32/Drivers/Button.hpp"
#include "base/I2CDatabase.hpp"
#include "ESP32/Drivers/I2C.hpp"

namespace LunokIoT {

/* default settings here */
// the desired time of "long press" can be: 0x0=4s, 0x1=6s(default), 0x2=8s, 0x3=10s (see REG36H:PEK bit [0]+[1] on datasheet)
#define AXP_DEFAULTS_PEK_POWERDOWN_TIME 0x0 /* set as 4s as default by this driver */




// normal pool i2c time in msecs
#define PEK_BUTTON_POOL_TIME 100
// when the button is pressed, increment the response time to (msecs):
#define PEK_BUTTON_SENSING_TIME 12


    class AXP202Driver : public DriverBaseClass {
        private:
            enum PEK_BUTTON {
                RELEASED=0,
                LONG=1,
                SHORT=2,
                MASK=3
            };
            
            enum I2C_REGISTER {
                // enable IRQ values
                IRQ_ENABLE_1=(0x40), /* bits: [X]=RESERVED,                 [1]=VBUS LOW,             [2]=VBUS REMOVED,      [3]=VBUS CONNECTED,      [4]=VBUS OVERVOLTAGE,    [5]=ACIN REMOVED,  [6]=ACIN CONNECTED,      [7]=ACIN OVERVOLTAGE  */
                IRQ_ENABLE_2=(0x41), /* bits: [0]=BATT LOW TEMP,            [1]=BATT OVERHEAT,        [2]=BATT CHARGED,      [3]=BATT CHARGING,       [4]=BATT EXIT ACTIVATE,  [5]=BATT ACTIVATE, [6]=BATT REMOVED,        [7]=BATT CONNECTED    */
                IRQ_ENABLE_3=(0x42), /* bits: [0]=PEK LONG,                 [1]=PEK SHORT,            [2]=LDO3 UNDERVOLTAGE, [3]=DC-DC3 UNDERVOLTAGE, [4]=DC-DC2 UNDERVOLTAGE, [X]=RESERVED,      [6]=CHARGE UNDERVOLTAGE, [7]=INTERNAL OVERHEAT */
                IRQ_ENABLE_4=(0x43), /* bits: [0]=AXP UNDERVOLTAGE LEVEL 2, [1]=UNDERVOLTAGE LEVEL 1, [2]=VBUS SESSION END,  [3]=VBUS SESION A/B,     [4]=VBUS INVALID,        [5]=VBUS VALID,    [6]=N_OE SHUTDOWN,       [7]=N_OE STARTUP      */
                IRQ_ENABLE_5=(0x44), /* bits: [0]=GPIO0 INPUT,              [1]=GPIO1 INPUT,          [2]=GPIO2 INPUT,       [3]=GPIO3 INPUT,         [X]=RESERVED,            [5]=PEK PRESS,     [6]=PEK RELEASED,        [7]=TIMER TIMEOUT     */

                IRQ_STATUS_1=(0x48),
                IRQ_STATUS_2=(0x49),
                IRQ_STATUS_3=(0x4A), /* bits: [0]=PEK LONG, [1]=PEK SHORT */
                IRQ_STATUS_4=(0x4B),
                IRQ_STATUS_5=(0x4C)
            };
        public:
            // PEK settings
            uint8_t lastVal = PEK_BUTTON::RELEASED;
            TickType_t lastEvent = 0;
            bool warnUserForPowerOff = true;

            AXP202Driver(I2CDriver *i2cHandler, i2c_port_t i2cport, uint32_t i2cfrequency, 
                        gpio_num_t i2csdagpio, gpio_num_t i2csclgpio,
                        uint8_t i2caddress=I2C_ADDR_AXP202);
            bool Loop();

            // i2c settings
            I2CDriver *i2cHandler;
            i2c_port_t port;
            uint32_t frequency;
            gpio_num_t sda;
            gpio_num_t scl;
            uint8_t address;
            void Init();
    };

}

#endif // ____LUNOKIOT___DRIVER_AXP192____
