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


    class AXP192Driver : public DriverBaseClass {
        private:
            enum PEK_BUTTON {
                RELEASED=0,
                LONG=1,
                SHORT=2,
                MASK=3
            };
            // https://github.com/m5stack/M5StickC-IDF/blob/master/components/m5stickc-idf/util/include/axp192_reg.h


            // https://github.com/tuupola/axp192/blob/master/axp192.h awesome!
            /*
            enum CONTROL {
                DCDC2_VOLTAGE=(0x23),
                DCDC1_VOLTAGE=(0x26),
                LDO23_VOLTAGE=(0x28),
                
                
                
                
                
                ADC_RATE_TS_PIN=(0x84),
            };*/
            
            enum I2C_REGISTER {
                // Power output control: 6 EXTEN, 4 DCDC2, 3 LDO3, 2 LDO2, 1 DCDC3, 0 DCDC1
                DCDC13_LDO23=(0x12),
                LDO23_VOLTAGE=(0x28),
                VBUS_IPSOUT_CHANNEL=(0x30),
                SHUTDOWN_BATTERY_CHGLED_CONTROL=(0x32),
                CHARGE_CONTROL_1=(0x33),
                BATTERY_CHARGE_CONTROL=(0x35),
                PEK_PARAMETHERS=(0x36),
                BATTERY_CHARGE_HIGH_TEMP=(0x39),
                //IRQ_STATUS_1=(0x44),
                //IRQ_STATUS_2=(0x45),
                IRQ_STATUS_3=(0x46), /* bits: [0]=PEK LONG, [1]=PEK SHORT */
                //IRQ_STATUS_4=(0x47),
                //ACIN_VOLTAGE=(0x56),
                //ACIN_CURRENT=(0x58),
                //VBUS_VOLTAGE=(0x5a),
                //VBUS_CURRENT=(0x5c),
                //TEMPERATURE=(0x5e),
                //BATTERY_POWER=(0x70),
                //BATTERY_VOLTAGE=(0x78),
                //CHARGE_CURRENT=(0x7a),
                //DISCHARGE_CURRENT=(0x7c),
                //APS_VOLTAGE=(0x7e),
                ADC_ENABLE_1=(0x82),
                GPIO0=(0x90),
                GPIO0_LDOIO0_VOLTAGE=(0x91)
                    
            };
        public:
            // PEK settings
            uint8_t lastVal = PEK_BUTTON::RELEASED;
            TickType_t lastEvent = 0;
            bool warnUserForPowerOff = true;

            AXP192Driver(I2CDriver *i2cHandler, i2c_port_t i2cport, uint32_t i2cfrequency, 
                        gpio_num_t i2csdagpio, gpio_num_t i2csclgpio,
                        uint8_t i2caddress=I2C_ADDR_AXP192);
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
