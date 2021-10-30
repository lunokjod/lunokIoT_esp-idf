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

extern SemaphoreHandle_t printfMutex;
#define printf(...) { \
    if ( pdTRUE == xSemaphoreTake(printfMutex, portMAX_DELAY) ) { \
        printf(__VA_ARGS__); \
        xSemaphoreGive(printfMutex); \
    } \
}
#define debug_write(...) printf(__VA_ARGS__);
//#define debug_headerC() debug_write("C   %08u %s:%d %s() ",xTaskGetTickCount(),__FILE__, __LINE__, __func__);
//#define debug_headerCPP() debug_write("%08u %s:%d %s ", xTaskGetTickCount(), __FILE__, __LINE__, __PRETTY_FUNCTION__);
#define debug_header() debug_write("%08u %s:%d > ", xTaskGetTickCount(), __FILENAMEONLY__, __LINE__);
////if ( pdTRUE == xSemaphoreTake(printfMutex, portMAX_DELAY) ) {
//xSemaphoreGive(printfMutex);
//}
#define debug_printferror(...) { \
    debug_write("%s", TERM_FG_RED); \
    debug_header(); \
    debug_write("ERROR: "); \
    debug_write(__VA_ARGS__); \
    debug_write("%s", TERM_RESET); \
    debug_write("\n"); \
    fflush(stdout); \
}
#define debug_printf(...) { \
    debug_write("%s", TERM_FG_GREY); \
    debug_header(); \
    debug_write("%s", TERM_RESET); \
    debug_write(__VA_ARGS__); \
    debug_write("\n"); \
    fflush(stdout); \
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
