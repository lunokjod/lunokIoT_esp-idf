#ifndef __LUNOKIOT__DEBUG___
#define __LUNOKIOT__DEBUG___

/*
#ifndef LUNOKIOT_SERIAL_SPEED
#define LUNOKIOT_SERIAL_SPEED 115200
#endif // LUNOKIOT_SERIAL_SPEED

#ifdef LUNOKIOT_DEBUG
extern SemaphoreHandle_t _lunokIoT_Serial_Debug_Semaphore;
extern HardwareSerial _lunokIoT_Serial_Debug;

#define DebugToSerial(speed) { \
    _lunokIoT_Serial_Debug_Semaphore = xSemaphoreCreateMutex(); \
    xSemaphoreTake(_lunokIoT_Serial_Debug_Semaphore, portMAX_DELAY); \
    _lunokIoT_Serial_Debug.begin(speed); \
    delay(50);  \
    xSemaphoreGive(_lunokIoT_Serial_Debug_Semaphore); \
}

#define debug_write(...) _lunokIoT_Serial_Debug.printf(__VA_ARGS__);

#define _DebugPrintCHeader() { \
    debug_write("%08lu %s:%d %s() ",millis(),__FILE__, __LINE__, __func__); \
}

#define _DebugOPrintCHeader() { \
    debug_write("%08lu %p %s:%d %s() ",millis(),this,__FILE__, __LINE__, __func__); \
}

#define DebugPrintf(...) { \
                xSemaphoreTake(_lunokIoT_Serial_Debug_Semaphore, portMAX_DELAY); \
                _DebugPrintCHeader(); \
                debug_write(__VA_ARGS__); \
                _lunokIoT_Serial_Debug.flush(); \
                xSemaphoreGive(_lunokIoT_Serial_Debug_Semaphore); \
}

#define DebugOPrintf(...) { \
                xSemaphoreTake(_lunokIoT_Serial_Debug_Semaphore, portMAX_DELAY); \
                _DebugOPrintCHeader(); \
                debug_write(__VA_ARGS__); \
                _lunokIoT_Serial_Debug.flush(); \
                xSemaphoreGive(_lunokIoT_Serial_Debug_Semaphore); \
}
#else

#define DebugToSerial()
#define debug_write(...)
#define DebugPrintf(...)
#define DebugOPrintf(...)

#endif // LUNOKIOT_DEBUG
*/
#endif // __LUNOKIOT__DEBUG___
