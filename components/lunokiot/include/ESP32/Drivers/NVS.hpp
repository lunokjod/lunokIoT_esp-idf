#ifndef ____LUNOKIOT___DRIVER_NVS____
#define ____LUNOKIOT___DRIVER_NVS____

#include "LunokIoT.hpp"
#include "../Driver.hpp"

namespace LunokIoT {
    class NVSDriver : public Driver {
        public:
            NVSDriver();
            bool Loop();
    };

}

#endif // ____LUNOKIOT___DRIVER_NVS____
