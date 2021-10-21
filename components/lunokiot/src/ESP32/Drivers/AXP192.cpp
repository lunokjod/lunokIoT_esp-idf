#include <driver/i2c.h>     // ESP32 i2c
#include <hal/gpio_types.h> // ESP32 gpio

#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/AXP192.hpp"
#include "ESP32/Drivers/Button.hpp"
#include "ESP32/Drivers/I2C.hpp"


using namespace LunokIoT;

void AXP192Driver::Init() {
    // M5StickC-Plus original init Arduino code and the lunokIoT equivalence
	
    /* Wire1.begin(21, 22);
	   Wire1.setClock(400000); */
    bool works = i2cHandler->GetI2CSession(port, frequency, sda, scl, address);
    //I2CInit();
    if ( false == works ) { return; }

    {
        // Set LDO2 & LDO3(TFT_LED & TFT) 3.0V
        /* Write1Byte(0x28, 0xcc); */

        // 0xcc = 11001100
        // LDO2 upper 4 bits (nibble) to 1100 = 12 = 1200mV + start 1800mV  = 3V
        // LDO3 lower 4 bits (nibble) to 1100
        // 1.8‐3.3V，100mV/step
        const uint8_t write_buf[2] = { I2C_REGISTER::LDO23_VOLTAGE, 0xcc };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Set LDO2 & LDO3 voltage to 3.0v (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // Set ADC to All Enable
        /* Write1Byte(0x82, 0xff); */
        const uint8_t write_buf[2] = { I2C_REGISTER::ADC_ENABLE_1, 0xff };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s All ADC enabled (0x%x) \n", this, name, write_buf[1]);
    }
    
    {
        // Bat charge voltage to 4.2, Current 100MA
        /* Write1Byte(0x33, 0xc0); */
        // 0xc0 = 0 10 1 1 010
        // bit    7 = Charging enable control bit, include internal access and external access 0:disable; 1:enable Default 1
        // bits 6~5 = Charging target voltage 00:4.1V； 01:4.15V； 10:4.2V； 11:4.36V Default 10 
        // bit    4 = Charge finished current 0: when charging current is less than 10% set value, finish charging 1: when charging current is less than 15% set value, finish charging Default 0
        // bits 3~0 = Internal charging current from 100mA to 1320mA
        const uint8_t write_buf[2] = { I2C_REGISTER::CHARGE_CONTROL_1, 0xc0 };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Bat charge voltage set to 4.2V 100mA (0x%x) \n", this, name, write_buf[1]);
    }
    
    {
        // Enable Bat,ACIN,VBUS,APS adc
        /* Write1Byte(0x82, 0xff); */
        const uint8_t write_buf[2] = { I2C_REGISTER::ADC_ENABLE_1, 0xff };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Enable Bat,ACIN,VBUS,APS adc (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // Enable Ext, LDO2, LDO3, DCDC1
    	/* Write1Byte(0x12, Read8bit(0x12) | 0x4D);	*/
        const uint8_t write_buf[2] = { I2C_REGISTER::DCDC13_LDO23, 0x4D };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Enable Ext, LDO2, LDO3, DCDC1 (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // 128ms power on, 4s power off
        /* Write1Byte(0x36, 0x0C); */
        const uint8_t write_buf[2] = { I2C_REGISTER::PEK_PARAMETHERS, 0x0C };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s PEK 128ms power on, 4s power off (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // Set RTC voltage to 3.3V
        /* Write1Byte(0x91, 0xF0);	*/
        const uint8_t write_buf[2] = { I2C_REGISTER::GPIO0_LDOIO0_VOLTAGE, 0xF0 };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Set RTC voltage to 3.3V (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // Set GPIO0 to LDO
        /* Write1Byte(0x90, 0x02); */
        const uint8_t write_buf[2] = { I2C_REGISTER::GPIO0, 0x02 };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Set GPIO0 to LDO (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // Disable vbus hold limit
        /* Write1Byte(0x30, 0x80); */
        const uint8_t write_buf[2] = { I2C_REGISTER::VBUS_IPSOUT_CHANNEL, 0x80 };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Disable vbus hold limit (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // Set temperature protection
        /* Write1Byte(0x39, 0xfc); */
        const uint8_t write_buf[2] = { I2C_REGISTER::BATTERY_CHARGE_HIGH_TEMP, 0xfc };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Set temperature protection (0x%x) \n", this, name, write_buf[1]);
    }

    {
        // Enable RTC BAT charge 
        /* Write1Byte(0x35, 0xa2); */
        const uint8_t write_buf[2] = { I2C_REGISTER::BATTERY_CHARGE_CONTROL, 0xa2 };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Enable RTC BAT charge (0x%x) \n", this, name, write_buf[1]);
    }


    {
        // Enable bat detection
        /* Write1Byte(0x32, 0x46); */
        const uint8_t write_buf[2] = { I2C_REGISTER::SHUTDOWN_BATTERY_CHGLED_CONTROL, 0x46 };
        esp_err_t res = i2c_master_write_to_device(port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
        if ( ESP_OK != res ) {
            printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
            i2cHandler->FreeI2CSession(port);
            return;
        }
        printf("%p %s Enable batt detection (0x%x) \n", this, name, write_buf[1]);
    }

    /*
    
    ScreenBreath(11);
    */
   i2cHandler->FreeI2CSession(port);
}
AXP192Driver::AXP192Driver(I2CDriver *i2cHandler, i2c_port_t i2cport, uint32_t i2cfrequency, gpio_num_t i2csdagpio, gpio_num_t i2csclgpio, uint8_t i2caddress): 
                                       DriverBaseClass((const char*)"(-) AXP192", (unsigned long)PEK_BUTTON_POOL_TIME), 
                                        i2cHandler(i2cHandler), port(i2cport), frequency(i2cfrequency), sda(i2csdagpio), scl(i2csclgpio), address(i2caddress) {
    printf("%p %s Setup (i2c port: %d, sda=%d scl=%d, addr:%d)\n", this, name, port, sda, scl, address);
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html
    
    this->Init();
    /*
    //@TODO my last implementation in arduino (must be ported here)
        this->_i2cWire.begin(sda, scl, freq);
        this->EnableEXTEN(true);
        this->EnableBACKUP(true);
        this->SetDCDC1Voltage(3300);
        this->SetDCDC2Voltage(0);
        this->SetDCDC3Voltage(0);
        this->SetLDO2Voltage(0); // no screen by default
        this->SetLDO3Voltage(0); // obious, no backlight :P
        this->SetGPIO0Voltage(-1);

        // hardcoded values :-(
        this->WriteByte(CONTROL::ADC_RATE_TS_PIN, 0b11110010);  // ADC 200Hz
        this->WriteByte(CONTROL::ADC_ENABLE_1, 0b11111111);  // ADC All Enable
        this->WriteByte(CONTROL::CHARGE_CONTROL_1, 0b11000000);  // Charge 4.2V, 100mA
        this->WriteByte(CONTROL::POWER_BUTTON, 0b00001100);  // 128ms, PW OFF 4S
        this->WriteByte(CONTROL::VBUS_IPSOUT_CHANNEL, 0b10000000);  // VBUS Open
        this->WriteByte(CONTROL::BATTERY_CHARGE_HIGH_TEMP, 0b11111100);  // Temp Protection
        this->WriteByte(SHUTDOWN_BATTERY_CHGLED_CONTROL, 0b00000100);  // Power Off 3.0V
*/

    //@TODO setup the AXP via i2c

    // PEK_PARAMETHERS must be set
    // i2c Step 1, configure:

    //@NOTE dummy message, this code must be moved to i2cButtonDriver, bitmask is filter i2c response to get button bool status (pressed/released)
    printf("%p (-) Button Setup (i2c reg=0x%x bitmask=0x%x)\n", this, I2C_REGISTER::IRQ_STATUS_3, PEK_BUTTON::MASK);
}

bool AXP192Driver::Loop() {
    // @NOTE the following code must be ported to some kind of inheritance of Button, and Device must expose it as normal GPIO button

    // i2c Step 1, configure:
    i2c_config_t i2cConf = { // --sda 21 --scl 22 --freq 400000
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (int)sda,
        .scl_io_num = (int)scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master {
            .clk_speed = I2C_MASTER_FREQ_HZ
        },
        .clk_flags = 0
    };
    esp_err_t res = i2c_param_config(I2C_NUM_0, &i2cConf);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_param_config ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        return true;
    }

    // Step 2, install driver:
    res = i2c_driver_install(I2C_NUM_0, i2cConf.mode , I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_driver_install ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        return true;
    }

    // Step 3, get the data
    uint8_t buttonStatus = 0;
    const uint8_t registerToAsk = I2C_REGISTER::IRQ_STATUS_3;

    res = i2c_master_write_read_device(I2C_NUM_0, I2C_ADDR_AXP192, &registerToAsk, 1, &buttonStatus, 1, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_master_write_read_device ERROR: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        i2c_driver_delete(I2C_NUM_0);
        return true;
    }


    // AXP192 irq ack
    const uint8_t write_buf[2] = { I2C_REGISTER::IRQ_STATUS_3, PEK_BUTTON::MASK };
    res = i2c_master_write_to_device(I2C_NUM_0, I2C_ADDR_AXP192, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        printf("%s:%d %s() i2c_master_write_to_device error: %s\n",__FILE__, __LINE__, __func__, esp_err_to_name(res));
        this->period = I2C_MASTER_TIMEOUT_MS; // slowdown the next iteration
        i2c_driver_delete(I2C_NUM_0);
        return true;
    }
    //}
    i2c_driver_delete(I2C_NUM_0);


    // logic button goes here:

    TickType_t thisEvent = xTaskGetTickCount();
    if ( buttonStatus != lastVal ) { // only if state of button changes

        printf("%p (-) Button(-1) ", this);
        if ( buttonStatus == PEK_BUTTON::RELEASED) {
            printf("Released");
        } else {
            printf("Pressed");
        }        
        if ( PEK_BUTTON::RELEASED == buttonStatus ) {
            // relased now
            TickType_t diffTime = thisEvent-lastEvent;
            printf(" %dms", diffTime);
        }
        
        if ( PEK_BUTTON::LONG == lastVal ) {
            printf(" long press event");
            this->period = PEK_BUTTON_POOL_TIME; // released, low pooling
        } else if ( PEK_BUTTON::SHORT == lastVal ) {
            printf(" click event");
            this->period = PEK_BUTTON_POOL_TIME; // released, low pooling
        } else {
            this->period = PEK_BUTTON_SENSING_TIME; // button pressed, high resolution poll to get accurated time
        }
        printf("\n");
        lastEvent = thisEvent;
    }
    lastVal = buttonStatus;

    // the most important, ¿is the user pushing the POWER BUTTON? send a warning!

    if ( PEK_BUTTON::LONG == buttonStatus ) { // do you like spagetti?
        if ( warnUserForPowerOff == true ) { // must warn user what are doing? (poweroff in 6 seconds!)
            printf("\n\n /!\\ /!\\ /!\\ WARNING!!! RELEASE the BUTTON before 6 seconds to AVOID POWERDOWN /!\\ /!\\ /!\\ \n\n\n");
            this->period = PEK_BUTTON_SENSING_TIME; // button pressed, high resolution poll to get accurated time
            fflush(stdout);
            return true;
        }
    } else if ( PEK_BUTTON::RELEASED == buttonStatus ) {
        if ( false == warnUserForPowerOff ) {
            printf("\nPOWEROFF procedure stopped due power button release :-)\n\n");
            warnUserForPowerOff = true;
        }
    }

    return true;
}
