
//@TODO https://github.com/espressif/esp-idf/blob/v4.3.1/examples/wifi/scan/main/scan.c

#include "LunokIoT.hpp"
#include "ESP32/Driver.hpp"
#include "ESP32/Drivers/WiFi.hpp"
#include <esp_log.h>

#include <stdio.h>
#include <string.h>
#include <esp_console.h>
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"


#define JOIN_TIMEOUT_MS (15000)
#define WIFI_MAX_RETRIES 3
#define WIFI_RECONNECT_TIMEOUT_MS (30000)
#define DEFAULT_SCAN_LIST_SIZE 32

const int CONNECTED_BIT = BIT0;
const int WIFI_FAIL_BIT = BIT1;

LunokIoT::WiFiDriver * LunokIoT::WiFiDriver::instance = nullptr;
EventGroupHandle_t LunokIoT::WiFiDriver::WifiEventGroup;

// https://github.com/espressif/esp-idf/blob/3e370c4296247b349aa3b9a0076c05b9946d47dc/examples/system/console/basic/main/cmd_wifi.c
void WiFiDriver::_EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    //LunokIoT::WiFiDriver *instance = static_cast<LunokIoT::WiFiDriver*>(arg);
    instance->EventHandler(arg, event_base, event_id, event_data);
}
int WiFiDriver::_Scan(int argc, char **argv) {
    return WiFiDriver::instance->Scan(argc, argv);
}

int WiFiDriver::_Init(int argc, char **argv) {
    return WiFiDriver::instance->Init(argc, argv);
}
int WiFiDriver::_Deinit(int argc, char **argv) {
    return WiFiDriver::instance->Deinit(argc, argv);
}

/* Initialize Wi-Fi as sta and set scan method */
int WiFiDriver::Scan(int argc, char **argv) {
    if ( true == initialized ) {
        printf("WiFi must be deinit before use Scan\n");
        return 1;
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    printf("@DEBUG A\n");
    fflush(stdout);
    esp_wifi_scan_start(NULL, true);
    printf("@DEBUG B\n");
    esp_wifi_scan_get_ap_records(&number, ap_info);
    printf("@DEBUG C\n");
    esp_wifi_scan_get_ap_num(&ap_count);
    printf("Total APs scanned = %u\n", ap_count);
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        printf("SSID \t\t%s\n", ap_info[i].ssid);
        printf("RSSI \t\t%d\n", ap_info[i].rssi);
        //printf("%s\n", WifiTypeToString(ap_info[i].authmode));
        if (ap_info[i].authmode != WIFI_AUTH_WEP) {
            //printf("Pairwise Cipher \t%s\n", WiFiDriver::WifiCypherTypeToString(ap_info[i].pairwise_cipher));
            //printf("Group Cipher \t%s\n", WiFiDriver::WifiCypherGroupToString(ap_info[i].group_cipher));
        }
        printf("Channel \t\t%d\n", ap_info[i].primary);
    }

    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_deinit();
    esp_event_loop_delete_default();
    esp_netif_deinit();
    


    return 0;
}

/** Arguments used by 'join' function */
static struct {
    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} join_args;

static bool wifi_join(const char *ssid, const char *pass, int timeout_ms) {
    
    wifi_config_t wifi_config = {};
    
    memcpy((char *)(wifi_config.sta.ssid), ssid, sizeof(wifi_config.sta.ssid));
    if (pass) {
        memcpy((char *)(wifi_config.sta.password), pass, sizeof(wifi_config.sta.password));
    }
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    //printf("FUCKING WAITING BITS\n");
    int bits = xEventGroupWaitBits(LunokIoT::WiFiDriver::WifiEventGroup, CONNECTED_BIT | WIFI_FAIL_BIT,
                                pdFALSE, pdFALSE, timeout_ms / portTICK_PERIOD_MS); // portMAX_DELAY);
    //printf("@DEBUG CONNECTED BIT: %s\n", (bits & CONNECTED_BIT)?"true":"false");
    //printf("@DEBUG WIFI FAIL BIT: %s\n", (bits & WIFI_FAIL_BIT)?"true":"false");
    if (bits & CONNECTED_BIT) { return true; }
    else if (bits & WIFI_FAIL_BIT) { return false; }
    else if (0 == bits ) { // timeout
    /*
        if ( LunokIoT::WiFiDriver::instance->wifiSTAIsConnected ) {
            printf("WIFI DHCP continues waiting on background\n");
            return true;
        } else {*/
        printf("WIFI ERROR: Timeout while trying to connect!\n");
        //}
        return false;
    }
    printf("@DEBUG WIFI Unknown event bits!! (bits: 0x%x)\n", bits);
    return false;
}


using namespace LunokIoT;

const char* WiFiDriver::WifiCypherGroupToString(int group_cipher) {

    switch (group_cipher) {
        case WIFI_CIPHER_TYPE_NONE:
            return "WIFI_CIPHER_TYPE_NONE";
            break;
        case WIFI_CIPHER_TYPE_WEP40:
            return "WIFI_CIPHER_TYPE_WEP40";
            break;
        case WIFI_CIPHER_TYPE_WEP104:
            return "WIFI_CIPHER_TYPE_WEP104";
            break;
        case WIFI_CIPHER_TYPE_TKIP:
            return "WIFI_CIPHER_TYPE_TKIP";
            break;
        case WIFI_CIPHER_TYPE_CCMP:
            return "WIFI_CIPHER_TYPE_CCMP";
            break;
        case WIFI_CIPHER_TYPE_TKIP_CCMP:
            return "WIFI_CIPHER_TYPE_TKIP_CCMP";
            break;
        default:
            return "WIFI_CIPHER_TYPE_UNKNOWN";
    }
}

const char* WiFiDriver::WifiCypherTypeToString(int pairwise_cipher) {
    switch (pairwise_cipher) {
        case WIFI_CIPHER_TYPE_NONE:
            return "WIFI_CIPHER_TYPE_NONE";
            break;
        case WIFI_CIPHER_TYPE_WEP40:
            return "WIFI_CIPHER_TYPE_WEP40";
            break;
        case WIFI_CIPHER_TYPE_WEP104:
            return "WIFI_CIPHER_TYPE_WEP104";
            break;
        case WIFI_CIPHER_TYPE_TKIP:
            return "WIFI_CIPHER_TYPE_TKIP";
            break;
        case WIFI_CIPHER_TYPE_CCMP:
            return "WIFI_CIPHER_TYPE_CCMP";
            break;
        case WIFI_CIPHER_TYPE_TKIP_CCMP:
            return "WIFI_CIPHER_TYPE_TKIP_CCMP";
            break;
        default:
            return "WIFI_CIPHER_TYPE_UNKNOWN";
    }

}
const char* WiFiDriver::WifiTypeToString(uint8_t mode) {
    switch(mode) {
        case WIFI_AUTH_OPEN:
            return "open";
        break;
        case WIFI_AUTH_WEP:
            return "WEP";
        break;
        case WIFI_AUTH_WPA_PSK:
            return "WPA PSK";
        break;
        case WIFI_AUTH_WPA2_PSK:
            return "WPA2 PSK";
        break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA/WPA2 PSK";
        break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2 Enterpirse";
        break;
        case WIFI_AUTH_WPA3_PSK:
            return "WPA3 PSK";
        break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2/WPA3 PSK";
        break;
        case WIFI_AUTH_WAPI_PSK:
            return "WAPI PSK";
        break;
        default:
            return "UNKNOWN";
    }
}

const char* WiFiDriver::WifiReasonToString(uint8_t errCode) {
    // https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/GeneralUtils.cpp
	if (errCode == ESP_OK) return "ESP_OK (received SYSTEM_EVENT_STA_GOT_IP event)";
	if (errCode == UINT8_MAX) return "Not Connected (default value)";

	switch ((wifi_err_reason_t) errCode) {
		case WIFI_REASON_UNSPECIFIED:
			return "WIFI_REASON_UNSPECIFIED";
		case WIFI_REASON_AUTH_EXPIRE:
			return "WIFI_REASON_AUTH_EXPIRE";
		case WIFI_REASON_AUTH_LEAVE:
			return "WIFI_REASON_AUTH_LEAVE";
		case WIFI_REASON_ASSOC_EXPIRE:
			return "WIFI_REASON_ASSOC_EXPIRE";
		case WIFI_REASON_ASSOC_TOOMANY:
			return "WIFI_REASON_ASSOC_TOOMANY";
		case WIFI_REASON_NOT_AUTHED:
			return "WIFI_REASON_NOT_AUTHED";
		case WIFI_REASON_NOT_ASSOCED:
			return "WIFI_REASON_NOT_ASSOCED";
		case WIFI_REASON_ASSOC_LEAVE:
			return "WIFI_REASON_ASSOC_LEAVE";
		case WIFI_REASON_ASSOC_NOT_AUTHED:
			return "WIFI_REASON_ASSOC_NOT_AUTHED";
		case WIFI_REASON_DISASSOC_PWRCAP_BAD:
			return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
		case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
			return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
		case WIFI_REASON_IE_INVALID:
			return "WIFI_REASON_IE_INVALID";
		case WIFI_REASON_MIC_FAILURE:
			return "WIFI_REASON_MIC_FAILURE";
		case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
			return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
		case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
			return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
		case WIFI_REASON_IE_IN_4WAY_DIFFERS:
			return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
		case WIFI_REASON_GROUP_CIPHER_INVALID:
			return "WIFI_REASON_GROUP_CIPHER_INVALID";
		case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
			return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
		case WIFI_REASON_AKMP_INVALID:
			return "WIFI_REASON_AKMP_INVALID";
		case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
			return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
		case WIFI_REASON_INVALID_RSN_IE_CAP:
			return "WIFI_REASON_INVALID_RSN_IE_CAP";
		case WIFI_REASON_802_1X_AUTH_FAILED:
			return "WIFI_REASON_802_1X_AUTH_FAILED";
		case WIFI_REASON_CIPHER_SUITE_REJECTED:
			return "WIFI_REASON_CIPHER_SUITE_REJECTED";
		case WIFI_REASON_BEACON_TIMEOUT:
			return "WIFI_REASON_BEACON_TIMEOUT";
		case WIFI_REASON_NO_AP_FOUND:
			return "WIFI_REASON_NO_AP_FOUND";
		case WIFI_REASON_AUTH_FAIL:
			return "WIFI_REASON_AUTH_FAIL";
		case WIFI_REASON_ASSOC_FAIL:
			return "WIFI_REASON_ASSOC_FAIL";
		case WIFI_REASON_HANDSHAKE_TIMEOUT:
			return "WIFI_REASON_HANDSHAKE_TIMEOUT";
		case WIFI_REASON_CONNECTION_FAIL:
			return "WIFI_REASON_CONNECTION_FAIL";
		case WIFI_REASON_AP_TSF_RESET:
			return "WIFI_REASON_AP_TSF_RESET";
		case WIFI_REASON_ROAMING:
			return "WIFI_REASON_ROAMING";
		default:
			return "Unknown ESP_ERR error";
	}
}

void WiFiDriver::EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        if (WIFI_EVENT_SCAN_DONE == event_id) {
            printf("WIFI Scan Done!\n");
            return;
        } else if (WIFI_EVENT_STA_START == event_id) { 
            //printf("WIFI Connecting to '%s'...\n", join_args.ssid->sval[0]);
            printf("WIFI Start connecting...\n");
            wifiSTAWithIP = false;
            wifiSTAIsConnected = false;
            reconnectRetries=1;
            esp_wifi_connect();
            return;
        } else if (WIFI_EVENT_STA_DISCONNECTED == event_id) {
            wifiSTAWithIP = false;
            wifiSTAIsConnected = false;
            wifi_event_sta_disconnected_t *disconnectData = static_cast<wifi_event_sta_disconnected_t*>(event_data);
            printf("WIFI disconnected from '%s' reason: '%s'\n", disconnectData->ssid, WifiReasonToString(disconnectData->reason));

            bool mustRetry = false;
            if ( WIFI_REASON_NO_AP_FOUND == disconnectData->reason ) {
                mustRetry = true;
            } else if ( WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT == disconnectData->reason ) {
                mustRetry = true;
            } else if ( WIFI_REASON_CONNECTION_FAIL == disconnectData->reason ) {
                mustRetry = true;
            } else if ( WIFI_REASON_UNSPECIFIED == disconnectData->reason ) {
                mustRetry = true;
            } else if ( WIFI_REASON_AUTH_LEAVE == disconnectData->reason ) {
                mustRetry = true;
            } else if ( WIFI_REASON_ASSOC_LEAVE == disconnectData->reason ) {
                    printf("WIFI Leaves connection.\n");
                    esp_wifi_set_mode(WIFI_MODE_NULL);
                    esp_wifi_stop();
                    return;
            }
            if ( mustRetry ) {
                if (reconnectRetries <= WIFI_MAX_RETRIES) {
                    printf("WIFI retry to connect (%d/%d)...\n", reconnectRetries, WIFI_MAX_RETRIES);
                    reconnectRetries++;
                    esp_wifi_connect();
                    return;
                }
                if ( this->waitingForReconnect ) {
                    printf("WIFI will try to connect in: %us...\n", WIFI_RECONNECT_TIMEOUT_MS/1000 );
                } else {
                    printf("WIFI connection retry counter: exausted\n");
                    xEventGroupSetBits(WifiEventGroup, WIFI_FAIL_BIT);
                }
                return;
            }
            xEventGroupSetBits(WifiEventGroup, WIFI_FAIL_BIT);
            // https://docs.espressif.com/projects/esp-idf/en/v4.3.1/esp32/api-reference/network/esp_wifi.html?highlight=wifi_reason_assoc_leave#_CPPv423WIFI_REASON_ASSOC_LEAVE
            printf("@DEBUG (must implement the reason) WIFI DISCONNECT from '%s' reason: '%s'(0x%x)\n", disconnectData->ssid, WifiReasonToString(disconnectData->reason), disconnectData->reason);
            
            esp_wifi_set_mode(WIFI_MODE_NULL);
            esp_wifi_stop();
            wifiSTAWithIP = false;
            wifiSTAIsConnected = false;
            printf("WIFI Enabled delayed connect...\n");
            waitingForReconnect = true;
            //esp_wifi_connect();
            return;
        } else if (WIFI_EVENT_STA_STOP == event_id) {
            printf("WIFI STA mode off\n");
            wifiSTAWithIP = false;
            wifiSTAIsConnected = false;
            //waitingForReconnect = false;
            return;
        } else if (WIFI_EVENT_STA_CONNECTED == event_id) {
            wifi_event_sta_connected_t *connectData = static_cast<wifi_event_sta_connected_t*>(event_data);
            printf("WIFI Connected with '%s'(%d), ",connectData->ssid, connectData->ssid_len);
            printf("BSSID: %x:%x:%x:%x:%x:%x, ", connectData->bssid[0], connectData->bssid[1], connectData->bssid[2], connectData->bssid[3], connectData->bssid[4], connectData->bssid[5]);
            printf("ch: %d, ", connectData->channel);
            printf("auth: %s\n", WiFiDriver::WifiTypeToString(connectData->authmode));
            esp_netif_dhcp_status_t dhcpcStatus = {};
            esp_err_t canGetDHCPCStatus = esp_netif_dhcpc_get_status(sta_netif, &dhcpcStatus);
            //const char * humanReadableResult = esp_err_to_name(canGetDHCPCStatus);
            //printf("WIFI DHCP client status: %s\n", humanReadableResult);
            if ( ESP_OK == canGetDHCPCStatus ) {
                printf("DHCP: ");
                switch((int)dhcpcStatus) {
                    case ESP_NETIF_DHCP_INIT:
                        printf("Initializing...");
                    break;
                    case ESP_NETIF_DHCP_STARTED:
                        printf("Started");
                    break;
                    case ESP_NETIF_DHCP_STOPPED:
                        printf("Stopped");
                    break;
                }
                printf("\n");
            }

            wifiSTAIsConnected = true;
            waitingForReconnect = false;
            xEventGroupClearBits(WifiEventGroup, WIFI_FAIL_BIT);
            return;
        } else if ( WIFI_EVENT_STA_BEACON_TIMEOUT == event_id) {
            printf("WIFI Station Beacon Timeout\n");
        }

        printf("UNMANAGED WIFI EVENT: event_id: 0x%x\n", event_id);
        xEventGroupSetBits(WifiEventGroup, WIFI_FAIL_BIT);
        return;

    } else if (event_base == IP_EVENT) {
        if ( event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *connectData = static_cast<ip_event_got_ip_t*>(event_data);
            const esp_netif_ip_info_t *ip_info = &connectData->ip_info;
            printf("WIFI received IP addr: " IPSTR ", ", IP2STR(&ip_info->ip));
            printf("netmask: " IPSTR ", ", IP2STR(&ip_info->netmask));
            printf("gateway: " IPSTR ", ", IP2STR(&ip_info->gw));
            printf("change: %s\n",connectData->ip_changed?"true":"false");
            wifiSTAWithIP = true;
            xEventGroupSetBits(WifiEventGroup, CONNECTED_BIT);
            return;
        } else if ( event_id == IP_EVENT_STA_LOST_IP) {
            printf("WIFI Lost IP!\n");
            wifiSTAWithIP = false;
            xEventGroupClearBits(WifiEventGroup, CONNECTED_BIT);
            return;
        }
    }
    printf("UNMANAGED EVENT: base: '%s' event:'0x%x'\n", event_base, event_id);

}
int WiFiDriver::Init(int argc, char **argv) {
    if (this->initialized) {
        //printf("ERROR: Already initialized\n");
        return 1;
    }
    this->Resume(); // start the task again
    esp_log_level_set("wifi", ESP_LOG_NONE);
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    esp_wifi_set_mode(WIFI_MODE_STA);
    this->initialized = true;
    
    return 0;
}


int WiFiDriver::Deinit(int argc, char **argv) {
    if (false == this->initialized) {
        //printf("ERROR: Isn't initialized\n");
        return 1;
    }

    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_stop();

    //esp_event_handler_instance_unregister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, &handler_test);

    //esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &handler_anyid);
    //esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &handler_ip);

    esp_wifi_deinit();
    
    //esp_netif_destroy(this->sta_netif);
    //esp_netif_destroy(this->ap_netif);
    //this->sta_netif = nullptr;
    //this->ap_netif = nullptr;

    //esp_event_loop_delete_default();
    //esp_netif_deinit();
    //vEventGroupDelete(WifiEventGroup);
    //WifiEventGroup = nullptr;
    //this->initialized = false;

    esp_log_level_set("wifi", ESP_LOG_NONE);
    this->Suspend(); // stops the tasks, no wifi to handle until Init again
    return 0;
}

int WiFiDriver::Disconnect(int argc, char **argv) {
    waitingForReconnect = false;
    if ( false == initialized ) {
        return 1;
    }
    if ( wifiSTAIsConnected ) {
        esp_wifi_disconnect();
    }

    wifiSTAIsConnected = false;
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_stop();
    return 0;
}

int WiFiDriver::Connect(int argc, char **argv) {
    if ( false == initialized ) {
        printf("@DEBUG not initialized on CONNECT\n");
        return 1;
    }
    if ( wifiSTAIsConnected ) {
        printf("@DEBUG already connected on CONNECT\n");
        return 2;
    }
    if ( waitingForReconnect ) {
        printf("@DEBUG waitingForReconnect enabled on CONNECT\n");
        return 3;
    }
    if ( wifiSTAWithIP ) {
        printf("@DEBUG wifiSTAWithIP enabled on CONNECT\n");
        return 4;
    }
    //printf("WIFI remaning retries on CONNECT %u\n", reconnectRetries);
    
    int nerrors = arg_parse(argc, argv, (void **) &join_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, join_args.end, argv[0]);
        printf("@DEBUG parse error on CONNECT\n");
        return 5;
    }
    //printf("WIFI Connecting to '%s'...\n", join_args.ssid->sval[0]);
    xEventGroupClearBits(WifiEventGroup, CONNECTED_BIT);
    xEventGroupClearBits(WifiEventGroup, WIFI_FAIL_BIT);
    /* set default value*/
    if (join_args.timeout->count == 0) {
        join_args.timeout->ival[0] = JOIN_TIMEOUT_MS;
    }

    bool connected = wifi_join(join_args.ssid->sval[0],
                               join_args.password->sval[0],
                               join_args.timeout->ival[0]);
    if (!connected) {
        //this->Disconnect();
        printf("WIFI ERROR: Unable to connect\n");
        //xEventGroupSetBits(WifiEventGroup, WIFI_FAIL_BIT);
        //esp_wifi_set_mode(WIFI_MODE_NULL);
        //esp_wifi_stop();
        return 6;
    }
    printf("WIFI Connected!\n");
    return 0;
}

int WiFiDriver::_Connect(int argc, char **argv) {
    return instance->Connect(argc, argv);
}

int WiFiDriver::_Disconnect(int argc, char **argv) {
    return instance->Disconnect(argc, argv);
}


WiFiDriver::WiFiDriver(): Driver((const char*)"(-) WiFi", 1000) {
    printf("%p %s Setup\n", this, this->name);
    WiFiDriver::instance = this; //@TODO this is UGLY
    const esp_console_cmd_t cmdInit = {
        .command = "wifi_init",
        .help = "Poweron the WiFi device",
        .hint = NULL,
        .func = &WiFiDriver::_Init,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdInit) );

    const esp_console_cmd_t cmdDeinit = {
        .command = "wifi_deinit",
        .help = "Poweroff the WiFi device",
        .hint = NULL,
        .func = &WiFiDriver::_Deinit,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdDeinit) );

    join_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    join_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    join_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    join_args.end = arg_end(2);

    const esp_console_cmd_t join_cmd = {
        .command = "wifi_join",
        .help = "Join WiFi AP as a station",
        .hint = NULL,
        .func = &WiFiDriver::_Connect,
        .argtable = &join_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&join_cmd) );

    const esp_console_cmd_t part_cmd = {
        .command = "wifi_leave",
        .help = "Shut down the current WiFi connection",
        .hint = NULL,
        .func = &WiFiDriver::_Disconnect,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&part_cmd) );


    const esp_console_cmd_t cmdScan = {
        .command = "wifi_scan",
        .help = "List AP's arround",
        .hint = NULL,
        .func = &WiFiDriver::_Scan,
        .argtable = nullptr
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmdScan) );


    WifiEventGroup = xEventGroupCreate();
    //xEventGroupClearBits(WifiEventGroup, WIFI_FAIL_BIT);
    //xEventGroupClearBits(WifiEventGroup, CONNECTED_BIT);
    esp_netif_init();

    esp_event_loop_create_default();
    //this->ap_netif = esp_netif_create_default_wifi_ap();
    this->sta_netif = esp_netif_create_default_wifi_sta();
    
    esp_event_handler_instance_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, &WiFiDriver::_EventHandler, this, &handler_test);
    //esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiDriver::_EventHandler, this, &handler_anyid);
    //esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiDriver::_EventHandler, this, &handler_ip);
    
    this->Suspend(); // stops the tasks, no wifi to handle until Init
}

bool WiFiDriver::Loop() {
    // delayed reconnect mechanism
    if ( waitingForReconnect ) {
        float divisor = 10.0;
        float totalSleep = 0.0;
        printf("WIFI Reconnect in %ds...\n", WIFI_RECONNECT_TIMEOUT_MS/1000);
        int iterations;
        float stepTimeMs = (WIFI_RECONNECT_TIMEOUT_MS/divisor);
        for(iterations=0;iterations < divisor; iterations++) {
            totalSleep+=stepTimeMs;
            //printf("WIFI Reconnect delay no: %d +%.2fs total: %.2fs\n", iterations, (stepTimeMs/1000.0), abs(totalSleep/1000));
            vTaskDelay(stepTimeMs / portTICK_PERIOD_MS);
            if ( false == waitingForReconnect ) { break; }
        }
        printf("@DEBUG LOOP iterations: %d divisor: %.2f\n", iterations, divisor);
        if ( iterations == divisor ) {
            printf("@DEBUG LOOP iterations done\n");
            // isn't solved yet?
            if ( waitingForReconnect ) {
                reconnectRetries=1;
                wifiSTAWithIP = false;
                wifiSTAIsConnected = false;
                
                esp_err_t resultSetStation = esp_wifi_set_mode(WIFI_MODE_STA);
                printf("WIFI Reconnect Started: %s\n", esp_err_to_name(resultSetStation));
                esp_wifi_start();
                esp_wifi_connect();
            }
        }
    }
    // @TODO other tasks here!

    /*
    printf("%p %s Loop > ", this, this->name);
    printf("@DEBUG ");
    printf("initialized: %s ", initialized?"true":"false");
    printf("wifiSTAWithIP: %s ", wifiSTAWithIP?"true":"false");
    printf("wifiSTAIsConnected: %s ", wifiSTAIsConnected?"true":"false");
    printf("\n");
    */
    return true;
}
