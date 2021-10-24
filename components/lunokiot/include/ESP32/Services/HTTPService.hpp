#ifndef ___LUNOKIOT__ESP32_SERVICE___HTTP___
#define ___LUNOKIOT__ESP32_SERVICE___HTTP___

#include "LunokIoT.hpp"
#include "base/ServiceTemplate.hpp"
#include <esp_http_server.h>
namespace LunokIoT {

    class HTTPService : public ServiceTemplate {
        public:
            HTTPService();
            bool Loop();
            httpd_handle_t handler;
            bool started = false;
    };

}

#endif // ___LUNOKIOT__ESP32_SERVICE___HTTP___
