#include "LunokIoT.hpp"
#include "ESP32/Service.hpp"
#include "ESP32/Services/NTPService.hpp"
#include "ESP32/Drivers/WiFi.hpp"
#include <esp_sntp.h>

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_attr.h>
#include <esp_sleep.h>
#include <nvs_flash.h>
#include <esp_console.h>

using namespace LunokIoT;

int NTPService::_DateCmd(int argc, char **argv) {
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    printf("Time: %s\n", strftime_buf);
    return 0;
}

NTPService::NTPService() : ServiceTemplate(((const char *)"(-) NTP Service"), 5000, 2000) {

//, unsigned long period=60000, uint32_t stackSize= configMINIMAL_STACK_SIZE)
    //Reconfigure task to = (unsigned long)CONFIG_LWIP_SNTP_UPDATE_DELAY
    debug_printf("Setup");
    setenv("TZ", "CST-2", 1);   //Spain mainland
    tzset();


    const esp_console_cmd_t cmdDate = {
        .command = "date",
        .help = "Get current date/time",
        .hint = NULL,
        .func = &NTPService::_DateCmd,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdDate) );

}

void time_sync_notification_cb(struct timeval *tv)
{
    debug_printf("Notification of a time synchronization event: ");
    time_t now = time(NULL);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    debug_printf("Time: %s\n", strftime_buf);
}

bool NTPService::Loop() {
    // silent until online
    bool connected = WiFiDriver::instance->IsOnline();
    if ( connected ) {
        debug_printf("Trying to sync with NTP...");
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "es.pool.ntp.org");
        sntp_set_time_sync_notification_cb(time_sync_notification_cb);
        sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
        sntp_init();


        this->Suspend();
        return true;
    }
    return true;
}
