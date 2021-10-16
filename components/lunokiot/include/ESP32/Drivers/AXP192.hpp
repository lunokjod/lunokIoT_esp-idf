#ifndef ____LUNOKIOT___DRIVER_AXP192____
#define ____LUNOKIOT___DRIVER_AXP192____

#include "LunokIoT.hpp"
#include "../Driver.hpp"
#include <hal/gpio_types.h>
#include "ESP32/Drivers/Button.hpp"

namespace LunokIoT {

    class AXP192Driver : public Driver {
        private:
            enum PEK_BUTTON {
                RELEASED=0,
                LONG=1,
                SHORT=2
            };
            // https://github.com/tuupola/axp192/blob/master/axp192.h awesome!
            enum CONTROL {
                /* Power output control: 6 EXTEN, 4 DCDC2, 3 LDO3, 2 LDO2, 1 DCDC3, 0 DCDC1 */
                DCDC13_LDO23=(0x12),
                DCDC2_VOLTAGE=(0x23),
                DCDC1_VOLTAGE=(0x26),
                LDO23_VOLTAGE=(0x28),
                VBUS_IPSOUT_CHANNEL=(0x30),
                SHUTDOWN_BATTERY_CHGLED_CONTROL=(0x32),
                CHARGE_CONTROL_1=(0x33),
                BATTERY_CHARGE_CONTROL=(0x35),
                POWER_BUTTON=(0x36),
                BATTERY_CHARGE_HIGH_TEMP=(0x39),
                ADC_ENABLE_1=(0x82),
                ADC_RATE_TS_PIN=(0x84),
                GPIO0=(0x90),
                GPIO0_LDOIO0_VOLTAGE=(0x91)
                
            };
            
            enum REGISTERS {
                IRQ_STATUS_3=(0x46), /* bits: [0]=PEK LONG, [1]=PEK SHORT */
                ACIN_VOLTAGE=(0x56),
                ACIN_CURRENT=(0x58),
                VBUS_VOLTAGE=(0x5a),
                VBUS_CURRENT=(0x5c),
                TEMPERATURE=(0x5e),
                BATTERY_POWER=(0x70),
                BATTERY_VOLTAGE=(0x78),
                CHARGE_CURRENT=(0x7a),
                DISCHARGE_CURRENT=(0x7c),
                APS_VOLTAGE=(0x7e)
                    
            };
        public:
            int lastVal = 3;
            TickType_t lastEvent = 0;

            AXP192Driver(gpio_num_t sdagpio, gpio_num_t sclgpio);
            bool Loop();
            gpio_num_t sda;
            gpio_num_t scl;
    };

}

#endif // ____LUNOKIOT___DRIVER_AXP192____
