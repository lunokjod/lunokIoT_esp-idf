#ifndef ____LUNOKIOT___DRIVER_AXP192____
#define ____LUNOKIOT___DRIVER_AXP192____

#include "LunokIoT.hpp"
#include "base/DriverTemplate.hpp"
#include <hal/gpio_types.h>
#include <driver/i2c.h>
#include "ESP32/Drivers/Button.hpp"
#include "base/I2CDatabase.hpp"
#include "ESP32/Drivers/I2C.hpp"

// https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/src/board/twatch2020_v3.h
#define TFT_WIDTH                   (240)
#define TFT_HEIGHT                  (240)

#define TWATCH_TFT_MISO             (GPIO_NUM_MAX)
#define TWATCH_TFT_MOSI             (GPIO_NUM_19)
#define TWATCH_TFT_SCLK             (GPIO_NUM_18)
#define TWATCH_TFT_CS               (GPIO_NUM_5)
#define TWATCH_TFT_DC               (GPIO_NUM_27)
#define TWATCH_TFT_RST              (GPIO_NUM_MAX)
#define TWATCH_TFT_BL               (GPIO_NUM_15)

#define TOUCH_SDA                   (GPIO_NUM_23)
#define TOUCH_SCL                   (GPIO_NUM_32)
#define TOUCH_INT                   (GPIO_NUM_38)
#define TOUCH_RST                   (GPIO_NUM_14)

#define SEN_SDA                     (GPIO_NUM_21)
#define SEN_SCL                     (GPIO_NUM_22)

#define RTC_INT_PIN                 (GPIO_NUM_37)
#define AXP202_INT                  (GPIO_NUM_35)
#define BMA423_INT1                 (GPIO_NUM_39)

#define TWATCH_2020_IR_PIN          (GPIO_NUM_13)


#define TWATCH_DAC_IIS_BCK          (GPIO_NUM_26)
#define TWATCH_DAC_IIS_WS           (GPIO_NUM_25)
#define TWATCH_DAC_IIS_DOUT         (GPIO_NUM_33)

#define MOTOR_PIN                   (GPIO_NUM_4)
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
            // https://www.tutorialspoint.com/cplusplus/cpp_bitwise_operators.htm
            //bool IRQ_ENABLE_1_BIT0_RESERVED = false;
            bool IRQ_ENABLE_1_BIT1_VBUS_LOW = false;
            bool IRQ_ENABLE_1_BIT2_VBUS_REMOVED = false;
            bool IRQ_ENABLE_1_BIT3_VBUS_CONNECTED = false;
            bool IRQ_ENABLE_1_BIT4_VBUS_OVERVOLTAGE = false;
            bool IRQ_ENABLE_1_BIT5_ACIN_REMOVED = false;
            bool IRQ_ENABLE_1_BIT6_ACIN_CONNECTED = false;
            bool IRQ_ENABLE_1_BIT7_ACIN_OVERVOLTAGE = false;

            bool IRQ_ENABLE_2_BIT0_BATT_LOW_TEMP = false;
            bool IRQ_ENABLE_2_BIT1_BATT_OVERHEAT = false;
            bool IRQ_ENABLE_2_BIT2_BATT_CHARGED = false;
            bool IRQ_ENABLE_2_BIT3_BATT_CHARGING = false;
            bool IRQ_ENABLE_2_BIT4_BATT_EXIT_ACTIVATE = false;
            bool IRQ_ENABLE_2_BIT5_BATT_ACTIVATE = false;
            bool IRQ_ENABLE_2_BIT6_BATT_REMOVED = false;
            bool IRQ_ENABLE_2_BIT6_BATT_CONNECTED = false;

            bool IRQ_ENABLE_3_BIT0_PEK_LONG = false;
            bool IRQ_ENABLE_3_BIT1_PEK_SHORT = false;
            bool IRQ_ENABLE_3_BIT2_LDO3_UNDERVOLTAGE = false;
            bool IRQ_ENABLE_3_BIT3_DC_DC3_UNDERVOLTAGE = false;
            bool IRQ_ENABLE_3_BIT4_DC_DC2_UNDERVOLTAGE = false;
            //bool IRQ_ENABLE_3_BIT5_RESERVED = false;
            bool IRQ_ENABLE_3_BIT6_CHARGE_UNDERVOLTAGE = false;
            bool IRQ_ENABLE_3_BIT7_INTERNAL_OVERHEAT = false;
 
            bool IRQ_ENABLE_4_BIT0_AXP_UNDERVOLTAGE_LEVEL2 = false;
            bool IRQ_ENABLE_4_BIT1_AXP_UNDERVOLTAGE_LEVEL1 = false;
            bool IRQ_ENABLE_4_BIT2_VBUS_SESSION_END = false;
            bool IRQ_ENABLE_4_BIT3_VBUS_SESSION_AB = false;
            bool IRQ_ENABLE_4_BIT4_VBUS_INVALID = false;
            bool IRQ_ENABLE_4_BIT5_VBUS_VALID = false;
            bool IRQ_ENABLE_4_BIT6_N_OE_SHUTDOWN = false;
            bool IRQ_ENABLE_4_BIT7_N_OE_STARTUP = false;

            bool IRQ_ENABLE_5_BIT0_GPIO0_INPUT = false;
            bool IRQ_ENABLE_5_BIT1_GPIO1_INPUT = false;
            bool IRQ_ENABLE_5_BIT2_GPIO2_INPUT = false;
            bool IRQ_ENABLE_5_BIT3_GPIO3_INPUT = false;
            //bool IRQ_ENABLE_5_BIT4_RESERVED = false;
            bool IRQ_ENABLE_5_BIT5_PEK_PRESS = false;
            bool IRQ_ENABLE_5_BIT6_PEK_RELEASED = false;
            bool IRQ_ENABLE_5_BIT7_TIMER_TIMEOUT = false;

            enum I2C_REGISTER {
                // enable IRQ values
                // default [7~0]=1101 1000
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
            bool ReadStatus();
            void DescribeStatus(uint8_t status[5]);
            bool Clearbits();
            bool StatusChangeActions();
            intr_handle_t handler;
            // i2c settings
            I2CDriver *i2cHandler;
            i2c_port_t port;
            uint32_t frequency;
            gpio_num_t sda;
            gpio_num_t scl;
            uint8_t address;
            void Init();
            SemaphoreHandle_t statusMutex = xSemaphoreCreateMutex();
    };

}

#endif // ____LUNOKIOT___DRIVER_AXP192____
