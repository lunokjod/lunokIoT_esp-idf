#ifndef ____LUNOKIOT___DRIVER_CONSOLE____
#define ____LUNOKIOT___DRIVER_CONSOLE____

#include "LunokIoT.hpp"
#include "../Driver.hpp"

namespace LunokIoT {
    class ConsoleDriver : public Driver {
        public:
            ConsoleDriver();
            bool Loop();
            static int Clear(int argc, char **argv);
    };

}

#endif // ____LUNOKIOT___DRIVER_CONSOLE____
