#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/I2C.hpp"
#include <esp_log.h>
#include <stdio.h>
#include "argtable3/argtable3.h"
#include "driver/i2c.h"
#include "esp_console.h"
#include "esp_log.h"

#include "base/I2CDatabase.hpp"

#define ESP32_I2C_PORTS 2
#define ESP32_I2C_FREE_TIMEOUT 1000;

// channel descriptors, see GetSession
lunokiot_i2c_channel_descriptor_t channels[ESP32_I2C_PORTS] = {};

#ifdef __cplusplus
extern "C" {
#endif



// https://raw.githubusercontent.com/espressif/esp-idf/b22c975a3bc5960b9a1efd631d3dd012998086f3/examples/peripherals/i2c/i2c_tools/main/cmd_i2ctools.c
/* cmd_i2ctools.c

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
#define I2C_MASTER_TX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define WRITE_BIT I2C_MASTER_WRITE  // I2C master write
#define READ_BIT I2C_MASTER_READ    // I2C master read
#define ACK_CHECK_EN 0x1            // I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0           // I2C master will not check ack from slave
#define ACK_VAL 0x0                 // I2C ack value
#define NACK_VAL 0x1                // I2C nack value
*/


// default i2c config
#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
static gpio_num_t i2c_gpio_sda = gpio_num_t(18);
static gpio_num_t i2c_gpio_scl = gpio_num_t(19);
#endif // CONFIG_LUNOKIOT_DEVICE_ESP32
#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE // defaults to Hy2.0-4P/GROVE 
static gpio_num_t i2c_gpio_sda = gpio_num_t(26);
static gpio_num_t i2c_gpio_scl = gpio_num_t(32);
#endif // CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_STICK_C_PLUS // defaulting to HAT pins
static gpio_num_t i2c_gpio_sda = gpio_num_t(0);
static gpio_num_t i2c_gpio_scl = gpio_num_t(26);
#endif // CONFIG_LUNOKIOT_DEVICE_M5STACK_STICK_C_PLUS
#ifdef CONFIG_LUNOKIOT_DEVICE_LILYGO_TWATCH_2020_V3
static gpio_num_t i2c_gpio_sda = gpio_num_t(21);
static gpio_num_t i2c_gpio_scl = gpio_num_t(22);
#endif // CONFIG_LUNOKIOT_DEVICE_LILYGO_TWATCH_2020_V3

static uint32_t i2c_frequency = I2C_MASTER_FREQ_HZ;
i2c_port_t i2c_port = I2C_DEFAULT_PORT; // defaults to 1, 0 is used by drivers

const char *i2cDatabase[128] = { nullptr };

esp_err_t i2c_get_port(int port, i2c_port_t *i2c_port) {
    if (port >= I2C_NUM_MAX) {
        printf("Wrong port number: %d\n", port);
        return ESP_FAIL;
    }
    switch (port) {
    case 0:
        *i2c_port = I2C_NUM_0;
        break;
    case 1:
        *i2c_port = I2C_NUM_1;
        break;
    default:
        *i2c_port = I2C_NUM_0;
        break;
    }
    return ESP_OK;
}

static esp_err_t i2c_master_driver_initialize(void)
{
    i2c_config_t conf = { }; 
    /* = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = i2c_gpio_sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = i2c_gpio_scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE
    };*/
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = i2c_gpio_sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = i2c_gpio_scl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = i2c_frequency;

    return i2c_param_config(i2c_port, &conf);
}

static struct {
    struct arg_int *port;
    struct arg_int *freq;
    struct arg_int *sda;
    struct arg_int *scl;
    struct arg_end *end;
} i2cconfig_args;

static int do_i2cconfig_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&i2cconfig_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, i2cconfig_args.end, argv[0]);
        return 0;
    }

    /* Check "--port" option */
    if (i2cconfig_args.port->count) {
        if (i2c_get_port(i2cconfig_args.port->ival[0], &i2c_port) != ESP_OK) {
            return 1;
        }
    }
    /* Check "--freq" option */
    if (i2cconfig_args.freq->count) {
        i2c_frequency = i2cconfig_args.freq->ival[0];
    }
    /* Check "--sda" option */
    i2c_gpio_sda = gpio_num_t(i2cconfig_args.sda->ival[0]);
    /* Check "--scl" option */
    i2c_gpio_scl = gpio_num_t(i2cconfig_args.scl->ival[0]);
    return 0;
}

static void register_i2cconfig(void)
{
    i2cconfig_args.port = arg_int0(NULL, "port", "<0|1>", "Set the I2C bus port number");
    i2cconfig_args.freq = arg_int0(NULL, "freq", "<Hz>", "Set the frequency(Hz) of I2C bus");
    i2cconfig_args.sda = arg_int1(NULL, "sda", "<gpio>", "Set the gpio for I2C SDA");
    i2cconfig_args.scl = arg_int1(NULL, "scl", "<gpio>", "Set the gpio for I2C SCL");
    i2cconfig_args.end = arg_end(2);
    const esp_console_cmd_t i2cconfig_cmd = {
        .command = "i2cconfig",
        .help = "Config I2C bus",
        .hint = NULL,
        .func = &do_i2cconfig_cmd,
        .argtable = &i2cconfig_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2cconfig_cmd));
}
static int do_i2cchannels_cmd(int argc, char **argv) {
    size_t offset = 0;
    for (;offset<ESP32_I2C_PORTS;offset++) {
        debug_printf("channel: %d", offset);
        if ( pdTRUE != xSemaphoreTake(channels[offset].useLock, 10) ) {
            debug_printf("channel: %d is in use", offset);
            //continue; // try next
        } else {
            xSemaphoreGive(channels[offset].useLock);
        }
        debug_printf("  channel port %d", channels[offset].port );
        debug_printf("  channel frequency %d", channels[offset].frequency );
        debug_printf("  channel sda %d", channels[offset].sda );
        debug_printf("  channel scl %d", channels[offset].scl );
        TickType_t lastUsed = channels[offset].lastUsed; 
        if ( 0 != lastUsed ) {
            lastUsed = xTaskGetTickCount() - channels[offset].lastUsed;;
        }
        debug_printf("  channel last use %dms", lastUsed );
    }
    return 0;
}

static int do_i2cdetect_cmd(int argc, char **argv) {
    bool deviceFoundAt[128] = { false };
    bool showBrief = false;
    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_master_driver_initialize();
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            fflush(stdout);
            address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 50 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK) {
                printf("%02x ", address);
                deviceFoundAt[address] = true;
                showBrief = true;
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }

    i2c_driver_delete(i2c_port);

    if ( showBrief ) {
        printf("\nList of devices found in i2c (sda: 0x%x, scl: 0x%x, speed: %dHz):\n", i2c_gpio_sda, i2c_gpio_scl, i2c_frequency);
        for(uint8_t current=0;current<128;current++) {
            if ( true == deviceFoundAt[current] ) {
                if ( nullptr != i2cDatabase[current] ) {
                    printf(" * %s\n", i2cDatabase[current]);
                } else {
                    printf(" * UNKNOWN (0x%x)\n", current);
                }
            }
        }
        printf("End of i2c device list\n");
    }
    return 0;
}

static void register_i2cchannels(void)
{
    const esp_console_cmd_t i2cchannels_cmd = {
        .command = "i2cchannels",
        .help = "Get lunokIoT channel queue information",
        .hint = NULL,
        .func = &do_i2cchannels_cmd,
        .argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2cchannels_cmd));
}



static void register_i2cdectect(void)
{
    const esp_console_cmd_t i2cdetect_cmd = {
        .command = "i2cdetect",
        .help = "Scan I2C bus for devices",
        .hint = NULL,
        .func = &do_i2cdetect_cmd,
        .argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2cdetect_cmd));
}

static struct {
    struct arg_int *chip_address;
    struct arg_int *register_address;
    struct arg_int *data_length;
    struct arg_end *end;
} i2cget_args;

static int do_i2cget_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&i2cget_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, i2cget_args.end, argv[0]);
        return 0;
    }

    /* Check chip address: "-c" option */
    int chip_addr = i2cget_args.chip_address->ival[0];
    /* Check register address: "-r" option */
    int data_addr = -1;
    if (i2cget_args.register_address->count) {
        data_addr = i2cget_args.register_address->ival[0];
    }
    /* Check data length: "-l" option */
    int len = 1;
    if (i2cget_args.data_length->count) {
        len = i2cget_args.data_length->ival[0];
    }
    uint8_t *data = (uint8_t*)malloc(len);

    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_master_driver_initialize();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    if (data_addr != -1) {
        i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
        i2c_master_start(cmd);
    }
    i2c_master_write_byte(cmd, chip_addr << 1 | READ_BIT, ACK_CHECK_EN);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, (i2c_ack_type_t)ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + len - 1, (i2c_ack_type_t)NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK) {
        for (int i = 0; i < len; i++) {
            printf("0x%02x ", data[i]);
            if ((i + 1) % 16 == 0) {
                printf("\r\n");
            }
        }
        if (len % 16) {
            printf("\r\n");
        }
    } else if (ret == ESP_ERR_TIMEOUT) {
        printf("Bus is busy\n");
    } else {
        printf("Read failed\n");
    }
    free(data);
    i2c_driver_delete(i2c_port);
    return 0;
}

static void register_i2cget(void)
{
    i2cget_args.chip_address = arg_int1("c", "chip", "<chip_addr>", "Specify the address of the chip on that bus");
    i2cget_args.register_address = arg_int0("r", "register", "<register_addr>", "Specify the address on that chip to read from");
    i2cget_args.data_length = arg_int0("l", "length", "<length>", "Specify the length to read from that data address");
    i2cget_args.end = arg_end(1);
    const esp_console_cmd_t i2cget_cmd = {
        .command = "i2cget",
        .help = "Read registers visible through the I2C bus",
        .hint = NULL,
        .func = &do_i2cget_cmd,
        .argtable = &i2cget_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2cget_cmd));
}

static struct {
    struct arg_int *chip_address;
    struct arg_int *register_address;
    struct arg_int *data;
    struct arg_end *end;
} i2cset_args;

static int do_i2cset_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&i2cset_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, i2cset_args.end, argv[0]);
        return 0;
    }

    /* Check chip address: "-c" option */
    int chip_addr = i2cset_args.chip_address->ival[0];
    /* Check register address: "-r" option */
    int data_addr = 0;
    if (i2cset_args.register_address->count) {
        data_addr = i2cset_args.register_address->ival[0];
    }
    /* Check data: "-d" option */
    int len = i2cset_args.data->count;

    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_master_driver_initialize();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    if (i2cset_args.register_address->count) {
        i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
    }
    for (int i = 0; i < len; i++) {
        i2c_master_write_byte(cmd, i2cset_args.data->ival[i], ACK_CHECK_EN);
    }
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK) {
        printf("Write OK\n");
    } else if (ret == ESP_ERR_TIMEOUT) {
        printf("Bus is busy\n");
    } else {
        printf("Write Failed\n");
    }
    i2c_driver_delete(i2c_port);
    return 0;
}

static void register_i2cset(void)
{
    i2cset_args.chip_address = arg_int1("c", "chip", "<chip_addr>", "Specify the address of the chip on that bus");
    i2cset_args.register_address = arg_int0("r", "register", "<register_addr>", "Specify the address on that chip to read from");
    i2cset_args.data = arg_intn(NULL, NULL, "<data>", 0, 256, "Specify the data to write to that data address");
    i2cset_args.end = arg_end(2);
    const esp_console_cmd_t i2cset_cmd = {
        .command = "i2cset",
        .help = "Set registers visible through the I2C bus",
        .hint = NULL,
        .func = &do_i2cset_cmd,
        .argtable = &i2cset_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2cset_cmd));
}

static struct {
    struct arg_int *chip_address;
    struct arg_int *size;
    struct arg_end *end;
} i2cdump_args;

static int do_i2cdump_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&i2cdump_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, i2cdump_args.end, argv[0]);
        return 0;
    }

    /* Check chip address: "-c" option */
    int chip_addr = i2cdump_args.chip_address->ival[0];
    /* Check read size: "-s" option */
    int size = 1;
    if (i2cdump_args.size->count) {
        size = i2cdump_args.size->ival[0];
    }
    if (size != 1 && size != 2 && size != 4) {
        printf("Wrong read size. Only support 1,2,4\n");
        return 1;
    }
    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    i2c_master_driver_initialize();
    uint8_t data_addr;
    uint8_t data[4];
    int32_t block[16];
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f"
           "    0123456789abcdef\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j += size) {
            fflush(stdout);
            data_addr = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
            i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, chip_addr << 1 | READ_BIT, ACK_CHECK_EN);
            if (size > 1) {
                i2c_master_read(cmd, data, size - 1, (i2c_ack_type_t)ACK_VAL);
            }
            i2c_master_read_byte(cmd, data + size - 1, (i2c_ack_type_t)NACK_VAL);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 50 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK) {
                for (int k = 0; k < size; k++) {
                    printf("%02x ", data[k]);
                    block[j + k] = data[k];
                }
            } else {
                for (int k = 0; k < size; k++) {
                    printf("XX ");
                    block[j + k] = -1;
                }
            }
        }
        printf("   ");
        for (int k = 0; k < 16; k++) {
            if (block[k] < 0) {
                printf("X");
            }
            if ((block[k] & 0xff) == 0x00 || (block[k] & 0xff) == 0xff) {
                printf(".");
            } else if ((block[k] & 0xff) < 32 || (block[k] & 0xff) >= 127) {
                printf("?");
            } else {
                printf("%c", block[k] & 0xff);
            }
        }
        printf("\r\n");
    }
    i2c_driver_delete(i2c_port);
    return 0;
}

static void register_i2cdump(void)
{
    i2cdump_args.chip_address = arg_int1("c", "chip", "<chip_addr>", "Specify the address of the chip on that bus");
    i2cdump_args.size = arg_int0("s", "size", "<size>", "Specify the size of each read");
    i2cdump_args.end = arg_end(1);
    const esp_console_cmd_t i2cdump_cmd = {
        .command = "i2cdump",
        .help = "Examine registers visible through the I2C bus",
        .hint = NULL,
        .func = &do_i2cdump_cmd,
        .argtable = &i2cdump_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&i2cdump_cmd));
}

void register_i2ctools(void)
{
    register_i2cconfig();
    register_i2cdectect();
    register_i2cget();
    register_i2cset();
    register_i2cdump();
    register_i2cchannels();
}

#ifdef __cplusplus
}
#endif

// https://static-cdn.m5stack.com/image/m5-docs_table/I2C_Address.pdf

void BuildI2CDatabase() { // database maybe are too for this x'D

    i2cDatabase[I2C_ADDR_BMM150] = "BMM150 (MAG)";
    i2cDatabase[I2C_ADDR_BMA423_ALT] = "BMA423 (MEMS) alt. addr.";
    i2cDatabase[I2C_ADDR_AXP202] = "AXP202 (PMU)"; // lilygo twatch 2020 v*?
    i2cDatabase[I2C_ADDR_MFRC522] = "MFRC522 (RFID)";
    i2cDatabase[I2C_ADDR_AXP192] = "AXP192 (PMU)"; // M5SickC/CPlus
    i2cDatabase[I2C_ADDR_FT6336] = "FT6336 (Touch)"; // lilygo
    i2cDatabase[I2C_ADDR_OLED_128x32] = "SSD1306 0,91' OLED 128x32";
    i2cDatabase[I2C_ADDR_OLED_128x64] = "OLED 128x64";
    i2cDatabase[I2C_ADDR_SHT30] = "SHT30";
    i2cDatabase[I2C_ADDR_BM8563] = "BM8563 (RTC)";
    i2cDatabase[I2C_ADDR_SGP30] = "SGP30";
    i2cDatabase[I2C_ADDR_MPU6886] = "MPU6886 (MEMS)"; //M5AtomLite/Matrix, M5StickC/CPlus
    i2cDatabase[I2C_ADDR_BMP280] = "BMP280";
}

using namespace LunokIoT;

I2CDriver::I2CDriver(): Driver((char*)"(-) I2C", 1000) {
    debug_printf("Setup");
    BuildI2CDatabase();
    register_i2ctools();

    // set mutex in all channels
    for (size_t offset = 0;offset<ESP32_I2C_PORTS;offset++) {
        channels[offset].useLock =  xSemaphoreCreateMutex();
    }
}

bool I2CDriver::Loop() {
    this->CleanupSessions();
    return true;
}

bool I2CDriver::GetSession(uint32_t i2cfrequency, 
                        gpio_num_t i2csdagpio, gpio_num_t i2csclgpio,
                        lunokiot_i2c_channel_descriptor_t &descriptor) {
    bool found = false;
    size_t offset = 0; // out of for-loop for get the offset outside
    for (;offset<ESP32_I2C_PORTS;offset++) {
        //debug_printf("@DEBUG trying channel: %d", offset);
        if ( pdTRUE != xSemaphoreTake(channels[offset].useLock, 10) ) {
            //debug_printf("@DEBUG channel: %d locked", offset);
            continue; // try next
        }
        // Got the semaphore!
        bool theSame = true; // be optimistic :)
        if ( 0 != channels[offset].frequency ) { // maybe not the same, but is free!!!
            if ( channels[offset].frequency != i2cfrequency ) { theSame = false; }
            else if ( channels[offset].sda != i2csdagpio ) { theSame = false; }
            else if ( channels[offset].scl != i2csclgpio ) { theSame = false; }
        }
        if ( not theSame ) { // not found equivalence (or free)...
            xSemaphoreGive(channels[offset].useLock); // free lock
            //debug_printf("@DEBUG Channel: %d is not the same", offset);
            continue; // try next
        }
        // yeah! one found free or reusable
        //debug_printf("@DEBUG Channel: %d free for use", offset);
        found = true;
        break;
    }
    // loop all the channels and any is free, time to do bad news....
    if ( not found ) {
        descriptor = {};
        debug_printferror("No free descriptors! please wait a while");
        return false;  // no free descriptors!!!
    }

    if ( 0 == channels[offset].frequency ) { // empty ones, fill it and initalize
        //debug_printf("@DEBUG Channel: %d must be initialized", offset);
        channels[offset].frequency = i2cfrequency;
        channels[offset].scl = i2csclgpio;
        channels[offset].sda = i2csdagpio;
        channels[offset].port = offset;
        // i2c Step 1, configure:
        i2c_config_t i2cConf = { // --sda 21 --scl 22 --freq 400000
            .mode = I2C_MODE_MASTER,
            .sda_io_num = (int)channels[offset].sda,
            .scl_io_num = (int)channels[offset].scl,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master {
                .clk_speed = channels[offset].frequency
            },
            .clk_flags = 0
        };
        bool initDone = true;
        esp_err_t res = i2c_param_config(channels[offset].port, &i2cConf);
        if ( ESP_OK != res ) {
            debug_printferror("i2c_param_config: %s\n",esp_err_to_name(res));
            initDone = false;
        }
        // Step 2, install driver:
        res = i2c_driver_install(channels[offset].port, i2cConf.mode , I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
        if ( ESP_OK != res ) {
            debug_printferror("i2c_driver_install: %s\n",esp_err_to_name(res));
            initDone = false;
        }
        // unable to start i2c, destroy descriptor
        if ( not initDone ) {
            channels[offset].frequency = 0;
            channels[offset].scl = gpio_num_t(0);
            channels[offset].sda = gpio_num_t(0);
            channels[offset].port = offset;
            xSemaphoreGive(channels[offset].useLock); // free lock
            debug_printferror("Unable to init new descriptor");
            return false;
        }
    }
    channels[offset].lastUsed = xTaskGetTickCount(); // update last use
    descriptor = channels[offset];
    //debug_printf("Obtained i2c channel %d", offset);
    return true; // at this point the mutex mark in use, must call FreeSession

}
void I2CDriver::CleanupSessions() {
    // iterate to close old unused descriptors
    size_t offset = 0;
    for (;offset<ESP32_I2C_PORTS;offset++) {
        if ( pdTRUE != xSemaphoreTake(channels[offset].useLock, 10) ) {
            //debug_printf("channel %d in use", offset);
            continue; // try next
        }
        TickType_t channelAge = channels[offset].lastUsed;
        if ( 0 == channelAge ) {
            xSemaphoreGive(channels[offset].useLock); // free lock
            continue;
        }
        TickType_t deadLine = channelAge + ESP32_I2C_FREE_TIMEOUT;
        TickType_t now = xTaskGetTickCount();
        if ( deadLine < now ) {
            esp_err_t res = i2c_driver_delete(channels[offset].port);
            if ( ESP_OK != res ) {
                debug_printferror("i2c_driver_delete ERROR: %s\n", esp_err_to_name(res));
            }
            channels[offset].frequency = 0;
            channels[offset].scl = gpio_num_t(0);
            channels[offset].sda = gpio_num_t(0);
            channels[offset].port = offset;
            channels[offset].lastUsed = 0;
            xSemaphoreGive(channels[offset].useLock); // free lock
            //debug_printf("Freed i2c channel %d due unused", offset);
            continue;
        }
        xSemaphoreGive(channels[offset].useLock); // free lock
        //debug_printf("i2c channel %d waiting for action", offset);
    }
}
bool I2CDriver::FreeSession(lunokiot_i2c_channel_descriptor_t &descriptor) {
    descriptor.lastUsed = xTaskGetTickCount(); // update timeout
    xSemaphoreGive(descriptor.useLock);
    this->CleanupSessions();
    return true;
}

bool I2CDriver::SetChar(lunokiot_i2c_channel_descriptor_t &descriptor, uint8_t address, const uint8_t i2cregister, const uint8_t value) {
    const uint8_t write_buf[2] = { i2cregister, value };
    esp_err_t res = i2c_master_write_to_device(descriptor.port, address, write_buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        debug_printferror("i2c_master_write_to_device: %s", esp_err_to_name(res));
        return false;
    }
    if ( value != write_buf[1]) {
        debug_printferror("check readed value failed '0x%x' isn't the setted: '0x%x'", write_buf[1], value);
        return false;
    }
    return true;
}

bool I2CDriver::GetChar(lunokiot_i2c_channel_descriptor_t &descriptor, uint8_t address, const uint8_t i2cregister, uint8_t &value) {
    esp_err_t res = i2c_master_write_read_device(descriptor.port, address, &i2cregister, 1, &value, 1, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
    if ( ESP_OK != res ) {
        debug_printferror("i2c_master_write_read_device: %s", esp_err_to_name(res));
        do_i2cchannels_cmd(0, nullptr);
        return false;
    }
    return true;
}
