#ifndef ____LUNOKIOT___DRIVER_CONSOLE____
#define ____LUNOKIOT___DRIVER_CONSOLE____
#include <esp_console.h>
#include <linenoise/linenoise.h>
#include <argtable3/argtable3.h>
#include <unistd.h>
#include <string.h>
#include <esp_system.h>
#include <esp_log.h>
#include <driver/uart.h>

#include "LunokIoT.hpp"
#include "../Driver.hpp"

namespace LunokIoT {
    class ConsoleDriver : public Driver {
        public:
            ConsoleDriver();
            bool Loop();
            static int Clear(int argc, char **argv);
            esp_console_repl_t *repl;
            esp_console_repl_config_t repl_config;
            esp_console_dev_uart_config_t uart_config;
            void EnableConsole();

    };

}

#endif // ____LUNOKIOT___DRIVER_CONSOLE____
