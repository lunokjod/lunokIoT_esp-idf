#ifndef ____LUNOKIOT___DRIVER_WIFI____
#define ____LUNOKIOT___DRIVER_WIFI____

#include "LunokIoT.hpp"
#include "../Driver.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"

namespace LunokIoT {
    class WiFiDriver : public Driver {
        public:
            
            // shit code 
            static WiFiDriver *instance;
            bool initialized = false;
            bool waitingForReconnect = false;
            size_t reconnectRetries = 0;
            bool wifiSTAWithIP = false; // is online? (full TCP/IP)
            bool wifiSTAIsConnected = false; // radio connection
            WiFiDriver();
            bool Loop();

            // esp32 Console component commands
            static int _Init(int argc, char **argv);
            static int _Deinit(int argc, char **argv);
            static int _Connect(int argc, char **argv);
            static int _Disconnect(int argc, char **argv);
            static int _Scan(int argc, char **argv);
            // here calls event loop
            static void _EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

            // wifi init
            int Init(int argc=0, char **argv=nullptr);
            int Deinit(int argc=0, char **argv=nullptr);
            // wifi connect
            int Connect(int argc, char **argv);
            int Disconnect(int argc=0, char **argv=nullptr);
            
            int Scan(int argc=0, char **argv=nullptr);

            // helpers
            static const char* WifiTypeToString(uint8_t mode);
            static const char* WifiCypherTypeToString(int pairwise_cipher);
            static const char* WifiCypherGroupToString(int group_cipher);
            static const char* WifiReasonToString(uint8_t errCode);
            bool IsOnline() { return wifiSTAWithIP && wifiSTAIsConnected; }
            // process the event
            void EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);    

            // event group
            static EventGroupHandle_t WifiEventGroup;
            esp_event_handler_instance_t handler_ip;
            esp_event_handler_instance_t handler_anyid;
            esp_event_handler_instance_t handler_test;
            esp_netif_t *ap_netif;
            esp_netif_t *sta_netif;
    };    
}

#endif // ____LUNOKIOT___DRIVER_WIFI____
