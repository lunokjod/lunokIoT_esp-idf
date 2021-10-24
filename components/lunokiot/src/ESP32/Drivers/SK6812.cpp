#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/SK6812.hpp"
//#include <nvs_flash.h>
#include <driver/rmt.h>

#include "esp_console.h"
#include "esp_err.h"
#include <argtable3/argtable3.h>

#define LED_STRIP_RMT_CLK_DIV 2

#define SK6812_T0H_NS   300
#define SK6812_T0L_NS   900
#define SK6812_T1H_NS   600
#define SK6812_T1L_NS   600


static rmt_item32_t sk6812_bit0 = { };
static rmt_item32_t sk6812_bit1 = { };
static SK6812Driver * sk6812Instance;
// based on: https://github.com/UncleRus/esp-idf-lib/tree/master/components/led_strip

static void IRAM_ATTR _rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
                                   size_t wanted_num, size_t *translated_size, size_t *item_num,
                                   const rmt_item32_t *bit0, const rmt_item32_t *bit1)
{
    if (!src || !dest)
    {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    while (size < src_size && num < wanted_num)
    {
        uint8_t b = *psrc;
        for (int i = 0; i < 8; i++)
        {
            // MSB first
            pdest->val = b & (1 << (7 - i)) ? bit1->val : bit0->val;
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

static void IRAM_ATTR sk6812_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size,
        size_t wanted_num, size_t *translated_size, size_t *item_num) {
    _rmt_adapter(src, dest, src_size, wanted_num, translated_size, item_num, &sk6812_bit0, &sk6812_bit1);
}

using namespace LunokIoT;

void SK6812Driver::SetLedColor(uint32_t color) {
    RGBColor nuColor;
    nuColor.raw = color;
    printf("R: %3d, ", nuColor.r);
    printf("G: %3d, ", nuColor.g);
    printf("B: %3d, ", nuColor.b);
    printf("RAW: 0x%x\n", nuColor.raw);
    this->currentLedColor.raw = color;
}


static struct {
    struct arg_int *color;
    struct arg_end *end;
} setLedColorArgs;

int SK6812Driver::_SetColorCmd(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &setLedColorArgs);
    if (nerrors != 0) {
        arg_print_errors(stderr, setLedColorArgs.end, argv[0]);
        return 1;
    }

    const uint32_t color = setLedColorArgs.color->ival[0];
    

    sk6812Instance->SetLedColor(color);

    return 0;
}

SK6812Driver::SK6812Driver(gpio_num_t gpio): Driver((const char*)"(-) SK6812", (unsigned long)1000), gpio(gpio) {
    debug_printf("Setup");
    sk6812Instance = this;

//void led_strip_install() {
    float ratio = (float)(APB_CLK_FREQ / LED_STRIP_RMT_CLK_DIV) / 1e09;
    sk6812_bit0.duration0 = ratio * SK6812_T0H_NS;
    sk6812_bit0.level0 = 1;
    sk6812_bit0.duration1 = ratio * SK6812_T0L_NS;
    sk6812_bit0.level1 = 0;
    sk6812_bit1.duration0 = ratio * SK6812_T1H_NS;
    sk6812_bit1.level0 = 1;
    sk6812_bit1.duration1 = ratio * SK6812_T1L_NS;
    sk6812_bit1.level1 = 0;

//esp_err_t led_strip_init(gpio_num_t gpio) {
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(this->gpio, (rmt_channel_t)0);
    config.clk_div = LED_STRIP_RMT_CLK_DIV;

    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);

    sample_to_rmt_t f = sk6812_rmt_adapter;
    rmt_translator_init(config.channel, f);



    setLedColorArgs.color = arg_int0(NULL, "color", "0xBBRRGG", "Set the M5Atom lite led color");
    setLedColorArgs.end = arg_end(2);

    const esp_console_cmd_t argSetColorCmd = {
        .command = "led_color",
        .help = "Set the M5Atom lite led color",
        .hint = NULL,
        .func = &SK6812Driver::_SetColorCmd,
        .argtable = &setLedColorArgs
    };
    esp_console_cmd_register(&argSetColorCmd);
    this->SetLedColor(0x010101);
}
bool SK6812Driver::Loop() {
    //printf("%s Driver Loop\n", this->name);

    //led_strip_flush(&strip);
    esp_err_t done = rmt_wait_tx_done((rmt_channel_t)0, pdMS_TO_TICKS(999)); // pdMS_TO_TICKS(1000));
    if ( ESP_OK == done ) {
        ets_delay_us(50);
        const uint8_t * memLocationOfLedColor = (const uint8_t *)&currentLedColor;
        // 3 = RGB
        rmt_write_sample((rmt_channel_t)0, memLocationOfLedColor, 3, false);
    }
    return true;
}
