#ifndef ___LUNOKIOT__MAIN___
#define ___LUNOKIOT__MAIN___
namespace LunokIoT {

}

#include "base/ANSI.hpp"

#include "Task.hpp"
#include "Device.hpp"
#include "Driver.hpp"
#include "Service.hpp"

#ifdef CONFIG_LUNOKIOT_DEBUG
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <string.h>

#define __FILENAMEONLY__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

extern SemaphoreHandle_t _debugMutex;
#define debug_write(...) printf(__VA_ARGS__);
//#define debug_headerC() debug_write("C   %08u %s:%d %s() ",xTaskGetTickCount(),__FILE__, __LINE__, __func__);
//#define debug_headerCPP() debug_write("%08u %s:%d %s ", xTaskGetTickCount(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
#define debug_header() debug_write("%08u %s:%d > ", xTaskGetTickCount(), __FILENAMEONLY__, __LINE__);
#define debug_printferror(...) { \
    if ( pdTRUE == xSemaphoreTake(_debugMutex, portMAX_DELAY) ) { \
        debug_write("%s", TERM_FG_RED); \
        debug_header(); \
        debug_write(__VA_ARGS__); \
        debug_write("%s", TERM_RESET); \
        debug_write("\n"); \
        fflush(stdout); \
        xSemaphoreGive(_debugMutex); \
    } \
}
#define debug_printf(...) { \
    if ( pdTRUE == xSemaphoreTake(_debugMutex, portMAX_DELAY) ) { \
        debug_write("%s", TERM_FG_GREY); \
        debug_header(); \
        debug_write("%s", TERM_RESET); \
        debug_write(__VA_ARGS__); \
        debug_write("\n"); \
        fflush(stdout); \
        xSemaphoreGive(_debugMutex); \
    } \
}
#else
#define debug_write(...)
#define debug_printf(...)
#define debug_printferror(...)
#endif //CONFIG_LUNOKIOT_DEBUG

namespace LunokIoT {
    // here comes everyting related with LunokIoT
    void Begin();
}

#endif // ___LUNOKIOT__MAIN___
