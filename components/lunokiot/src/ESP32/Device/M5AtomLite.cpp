#include <LunokIoT.hpp>
#include "ESP32/Device.hpp"
#include "ESP32/Device/M5AtomLite.hpp"
#include "ESP32/Driver/NVS.hpp"
#include "ESP32/Driver/Console.hpp"
#include "ESP32/Driver/I2C.hpp"
#include "ESP32/Service/NTPService.hpp"

#include <freertos/task.h>
#include <esp_spi_flash.h>
#include <esp_console.h>
#include <argtable3/argtable3.h>
#include <driver/uart.h>
#include <driver/rtc_io.h>
#include <ctype.h>
#include <unistd.h>
#include <esp_sleep.h>

using namespace LunokIoT;

static struct {
    struct arg_int *wakeup_time;
    struct arg_int *wakeup_gpio_num;
    struct arg_int *wakeup_gpio_level;
    struct arg_end *end;
} light_sleep_args;

/** 'deep_sleep' command puts the chip into deep sleep mode */

static struct {
    struct arg_int *wakeup_time;
#if SOC_PM_SUPPORT_EXT_WAKEUP
    struct arg_int *wakeup_gpio_num;
    struct arg_int *wakeup_gpio_level;
#endif
    struct arg_end *end;
} deep_sleep_args;

/** 'restart' command restarts the program */

int M5AtomLiteDevice::Restart(int argc, char **argv) {
    printf("\n\n M5AtomLiteDevice *** RESTART NOW ***\n\n\n");
    fflush(stdout);
    esp_restart();
    return 0;
}
/** 'free' command prints available heap memory */

int M5AtomLiteDevice::FreeMem(int argc, char **argv) {
    printf("%d\n", esp_get_free_heap_size());
    return 0;
}
/* 'heap' command prints minumum heap size */
int M5AtomLiteDevice::Heap(int argc, char **argv) {
    uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    printf("min heap size: %u\n", heap_size);
    return 0;
}
int M5AtomLiteDevice::Tasks(int argc, char **argv) {
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = (char*)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        printf("failed to allocate buffer for vTaskList output");
        return 1;
    }
    //fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
    printf(TERM_BOLD);
    printf(TERM_BG_BLUE);
    printf(TERM_FG_WHITE);
    //printf("Task Name\tStatus\tPrio\tHWM\tTask#\n");
    printf("  Task Name       Status  Prio    HWM     Task#   Affinity   ");
//#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    printf(TERM_RESET);
    fputs("\n", stdout);

    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    printf(TERM_FG_GREY);
    printf("Tasks are reported as blocked (‘B’), ready (‘R’), deleted (‘D’) or suspended (‘S’).");
    printf(TERM_RESET);
    printf("\n");
    return 0;
}
// https://docs.espressif.com/projects/esp-idf/en/v4.3.1/esp32/api-reference/system/freertos.html?highlight=taskgetidleruntimecounter#_CPPv427ulTaskGetIdleRunTimeCounterv
// https://www.freertos.org/a00021.html#vTaskGetRunTimeStats
int M5AtomLiteDevice::Scheduler(int argc, char **argv) {
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = (char*)malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        printf("failed to allocate buffer for vTaskGetRunTimeStats output");
        return 1;
    }
    // While uxTaskGetSystemState() and vTaskGetRunTimeStats() writes the total execution time of each task into a buffer, ulTaskGetIdleRunTimeCounter() returns the total execution time of just the idle task.
    vTaskGetRunTimeStats(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    
    uint32_t idleTime = ulTaskGetIdleRunTimeCounter() / 1000000;
    uint32_t runTime = portGET_RUN_TIME_COUNTER_VALUE() / 100000000;
    printf("-> Idle time: %us  Run time: %us\n", idleTime, runTime);
    // -> Idle ticks: 53736270 time: 537362 runCount: 141004407 tickPeriod: 1

    return 0;
}

int M5AtomLiteDevice::Info(int argc, char **argv) {
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    printf("IDF Version:%s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", chip_info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    printf("\tcores:%d\r\n", chip_info.cores);
    printf("\tfeature: WiFi%s%s%s%s%d%s\r\n",
           chip_info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           chip_info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           chip_info.features & CHIP_FEATURE_BT ? "/BT" : "",
           chip_info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", chip_info.revision);
    /*
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    */
    printf("Minimum free heap size: %dKb of %dKb\n", esp_get_minimum_free_heap_size()/1024, 520);
    return 0;    
}

int M5AtomLiteDevice::LightSleep(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &light_sleep_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, light_sleep_args.end, argv[0]);
        return 1;
    }
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    if (light_sleep_args.wakeup_time->count) {
        uint64_t timeout = 1000ULL * light_sleep_args.wakeup_time->ival[0];
        printf("Enabling timer wakeup, timeout=%lluus\n", timeout);
        ESP_ERROR_CHECK( esp_sleep_enable_timer_wakeup(timeout) );
    }
    int io_count = light_sleep_args.wakeup_gpio_num->count;
    if (io_count != light_sleep_args.wakeup_gpio_level->count) {
        printf("Should have same number of 'io' and 'io_level' arguments\n");
        return 1;
    }
    for (int i = 0; i < io_count; ++i) {
        int io_num = light_sleep_args.wakeup_gpio_num->ival[i];
        int level = light_sleep_args.wakeup_gpio_level->ival[i];
        if (level != 0 && level != 1) {
            printf("Invalid wakeup level: %d\n", level);
            return 1;
        }
        printf("Enabling wakeup on GPIO%d, wakeup on %s level\n",
                 io_num, level ? "HIGH" : "LOW");

        ESP_ERROR_CHECK( gpio_wakeup_enable((gpio_num_t)io_num, level ? GPIO_INTR_HIGH_LEVEL : GPIO_INTR_LOW_LEVEL) );
    }
    if (io_count > 0) {
        ESP_ERROR_CHECK( esp_sleep_enable_gpio_wakeup() );
    }
    if (CONFIG_ESP_CONSOLE_UART_NUM >= 0 && CONFIG_ESP_CONSOLE_UART_NUM <= UART_NUM_1) {
        printf("Enabling UART wakeup (press ENTER to exit light sleep)\n");
        ESP_ERROR_CHECK( uart_set_wakeup_threshold(CONFIG_ESP_CONSOLE_UART_NUM, 3) );
        ESP_ERROR_CHECK( esp_sleep_enable_uart_wakeup(CONFIG_ESP_CONSOLE_UART_NUM) );
    }
    fflush(stdout);
    fsync(fileno(stdout));
    esp_light_sleep_start();
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    const char *cause_str;
    switch (cause) {
    case ESP_SLEEP_WAKEUP_GPIO:
        cause_str = "GPIO";
        break;
    case ESP_SLEEP_WAKEUP_UART:
        cause_str = "UART";
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        cause_str = "timer";
        break;
    default:
        cause_str = "unknown";
        printf("%d\n", cause);
    }
    printf("Woke up from: %s\n", cause_str);
    return 0;
}

int M5AtomLiteDevice::DeepSleep(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &deep_sleep_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, deep_sleep_args.end, argv[0]);
        return 1;
    }
    if (deep_sleep_args.wakeup_time->count) {
        uint64_t timeout = 1000ULL * deep_sleep_args.wakeup_time->ival[0];
        printf("Enabling timer wakeup, timeout=%lluus\n", timeout);
        ESP_ERROR_CHECK( esp_sleep_enable_timer_wakeup(timeout) );
    }

#if SOC_PM_SUPPORT_EXT_WAKEUP
    if (deep_sleep_args.wakeup_gpio_num->count) {
        int io_num = deep_sleep_args.wakeup_gpio_num->ival[0];
        if (!esp_sleep_is_valid_wakeup_gpio((gpio_num_t)io_num)) {
            printf("GPIO %d is not an RTC IO\n", io_num);
            return 1;
        }
        int level = 0;
        if (deep_sleep_args.wakeup_gpio_level->count) {
            level = deep_sleep_args.wakeup_gpio_level->ival[0];
            if (level != 0 && level != 1) {
                printf("Invalid wakeup level: %d\n", level);
                return 1;
            }
        }
        printf("Enabling wakeup on GPIO%d, wakeup on %s level\n",
                 io_num, level ? "HIGH" : "LOW");

        ESP_ERROR_CHECK( esp_sleep_enable_ext1_wakeup(1ULL << io_num, (esp_sleep_ext1_wakeup_mode_t)level) );
        printf("GPIO wakeup from deep sleep currently unsupported on ESP32-C3\n");
    }
#endif // SOC_PM_SUPPORT_EXT_WAKEUP

#if CONFIG_IDF_TARGET_ESP32
    rtc_gpio_isolate(GPIO_NUM_12);
#endif //CONFIG_IDF_TARGET_ESP32

    esp_deep_sleep_start();
}

void M5AtomLiteDevice::RegisterConsoleCommands(void) {
    const esp_console_cmd_t cmdRestart = {
        .command = "restart",
        .help = "Restart the device",
        .hint = NULL,
        .func = &M5AtomLiteDevice::Restart,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdRestart) );
    const esp_console_cmd_t cmdFreeHeap = {
        .command = "free",
        .help = "Get the current size of free heap memory",
        .hint = NULL,
        .func = &M5AtomLiteDevice::FreeMem,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdFreeHeap) );
    const esp_console_cmd_t cmdHeap = {
        .command = "heap",
        .help = "Get minimum size of free heap memory that was available during program execution",
        .hint = NULL,
        .func = &M5AtomLiteDevice::Heap,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdHeap) );
    const esp_console_cmd_t cmdTasks = {
        .command = "tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &M5AtomLiteDevice::Tasks,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdTasks) );

    const esp_console_cmd_t cmdScheduler = {
        .command = "sched",
        .help = "Get information about scheduler",
        .hint = NULL,
        .func = &M5AtomLiteDevice::Scheduler,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdScheduler) );


    const esp_console_cmd_t cmdInfo = {
        .command = "info",
        .help = "Get information about the device",
        .hint = NULL,
        .func = &M5AtomLiteDevice::Info,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdInfo) );

    light_sleep_args.wakeup_time =
        arg_int0("t", "time", "<t>", "Wake up time, ms");
    light_sleep_args.wakeup_gpio_num =
        arg_intn(NULL, "io", "<n>", 0, 8,
                 "If specified, wakeup using GPIO with given number");
    light_sleep_args.wakeup_gpio_level =
        arg_intn(NULL, "io_level", "<0|1>", 0, 8, "GPIO level to trigger wakeup");
    light_sleep_args.end = arg_end(3);

    const esp_console_cmd_t cmdLightSleep = {
        .command = "light_sleep",
        .help = "Enter light sleep mode. "
        "Two wakeup modes are supported: timer and GPIO. "
        "Multiple GPIO pins can be specified using pairs of "
        "'io' and 'io_level' arguments. "
        "Will also wake up on UART input.",
        .hint = NULL,
        .func = &M5AtomLiteDevice::LightSleep,
        .argtable = &light_sleep_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdLightSleep) );

    int num_args = 1;
    deep_sleep_args.wakeup_time =
        arg_int0("t", "time", "<t>", "Wake up time, ms");
#if SOC_PM_SUPPORT_EXT_WAKEUP
    deep_sleep_args.wakeup_gpio_num =
        arg_int0(NULL, "io", "<n>",
                 "If specified, wakeup using GPIO with given number");
    deep_sleep_args.wakeup_gpio_level =
        arg_int0(NULL, "io_level", "<0|1>", "GPIO level to trigger wakeup");
    num_args += 2;
#endif
    deep_sleep_args.end = arg_end(num_args);

    const esp_console_cmd_t cmdDeepSleep = {
        .command = "deep_sleep",
        .help = "Enter deep sleep mode. "
#if SOC_PM_SUPPORT_EXT_WAKEUP
        "Two wakeup modes are supported: timer and GPIO. "
#else
        "Timer wakeup mode is supported. "
#endif
        "If no wakeup option is specified, will sleep indefinitely.",
        .hint = NULL,
        .func = &M5AtomLiteDevice::DeepSleep,
        .argtable = &deep_sleep_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdDeepSleep) );
}

M5AtomLiteDevice::M5AtomLiteDevice(): Device((const char*)"(-) M5AtomLite") {
    printf("%p %s Setup\n", this, this->name);
    this->_period = 2000;

    this->console = new ConsoleDriver();
    this->RegisterConsoleCommands();
    this->nvs = new NVSDriver();
    this->i2c = new I2CDriver();
    this->wifi = new WiFiDriver();
    this->ntp = new NTPService();

    printf("M5AtomLiteDevice Device Seup ends here\n");
}
#define LUNOKIOT_LOG_MARK_TIME_MS 60000

TickType_t nextRefresh = 0;
bool M5AtomLiteDevice::Loop() {
    /*
    TickType_t now = (xTaskGetTickCount()/portTICK_RATE_MS);
    if ( nextRefresh < now ) {
        printf("%08ul > --- MARK --- (every %ds)\n", now, LUNOKIOT_LOG_MARK_TIME_MS / 1000);
        nextRefresh = (xTaskGetTickCount()/portTICK_RATE_MS) + LUNOKIOT_LOG_MARK_TIME_MS; // do mark every minute
    }*/
    return true;
}
