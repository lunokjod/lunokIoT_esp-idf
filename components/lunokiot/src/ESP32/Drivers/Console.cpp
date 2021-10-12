#include <LunokIoT.hpp>
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/Console.hpp"

#include <esp_console.h>
#include <linenoise/linenoise.h>
#include <argtable3/argtable3.h>
#include <unistd.h>
#include <string.h>

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"


using namespace LunokIoT;

int ConsoleDriver::Clear(int argc, char **argv) {
    linenoiseClearScreen();
    return 0;
}

ConsoleDriver::ConsoleDriver(): Driver((const char*)"(-) Console", (unsigned long)-1) {
    printf("%p %s Setup\n", this, this->name);

    /*
    // Initialize the console 
    esp_console_config_t console_config = ESP_CONSOLE_CONFIG_DEFAULT();
    console_config.max_cmdline_length = 1024;
    {
            .max_cmdline_args = 8,
            .max_cmdline_length = 1024,
            .hint_color = atoi(LOG_COLOR_CYAN)
    };
    //ESP_ERROR_CHECK( esp_console_init(&console_config));*/

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = "lunokIoT> ";

    esp_console_register_help_command();
    const esp_console_cmd_t cmdClear = {
        .command = "clear",
        .help = "Clear the screen",
        .hint = NULL,
        .func = &ConsoleDriver::Clear,
        .argtable = nullptr
    };
    esp_console_cmd_register(&cmdClear);

    fflush(stdout);
    fsync(fileno(stdout));
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);

    esp_console_new_repl_uart(&uart_config, &repl_config, &repl);

    esp_console_start_repl(repl);

    /* Set command history size */
    linenoiseHistorySetMaxLen(20);

    /* Set command maximum length */
    //linenoiseSetMaxLineLen(1024);
    linenoiseSetMultiLine(1);
}

bool ConsoleDriver::Loop() {
    //printf("%p %s Loop\n", this, this->name);
    return true;
}
