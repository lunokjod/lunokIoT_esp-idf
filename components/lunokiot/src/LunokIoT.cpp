#include <iostream>

#ifdef CONFIG_LUNOKIOT_DEBUG
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
SemaphoreHandle_t _debugMutex = xSemaphoreCreateMutex();
#endif // CONFIG_LUNOKIOT_DEBUG

#include "LunokIoT.hpp"
#include "base/ANSI.hpp"

#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
#include "ESP32/Devices/ESP32.hpp"
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5AtomLite.hpp"
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_STICK_C_PLUS
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/M5StickCPlus.hpp"
#endif

#ifdef CONFIG_LUNOKIOT_DEVICE_LILYGO_TWATCH_2020_V3
#include "ESP32/Devices/ESP32.hpp"
#include "ESP32/Devices/LilygoTWatch2020v3.hpp"
#endif


void LunokIoT::Begin() {

#ifdef CONFIG_LUNOKIOT_DEBUG
    debug_printf("%sWARNING! DEBUG MODE ENABLED%s",TERM_FG_RED,TERM_RESET);
#endif //  CONFIG_LUNOKIOT_DEBUG

#ifdef CONFIG_LUNOKIOT_DEVICE_ESP32
    debug_printf("lunokIoT device: ESP32 generic");
    new ESP32Device();
    return;
#endif
#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_ATOM_LITE
    debug_printf("lunokIoT device: M5Atom Lite");
    new M5AtomLiteDevice();
    return;
#endif
#ifdef CONFIG_LUNOKIOT_DEVICE_M5STACK_STICK_C_PLUS
    debug_printf("lunokIoT device: M5StickC Plus");
    new M5StickCPlusDevice();
    return;
#endif
#ifdef CONFIG_LUNOKIOT_DEVICE_LILYGO_TWATCH_2020_V3
    debug_printf("lunokIoT device: LilyGo T-Watch 2020v3");
    new LilygoTWatch2020v3Device();
    return;
#endif
}
