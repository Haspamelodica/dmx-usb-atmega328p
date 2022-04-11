// for SPLIT_MODE
#include "dmxusb-config-common.h"

#if IS_DMX_CLIENT
    #if IS_DMX_SERVER
        #error "Sketch mustn't be client and server at the same time. \
        Only define IS_DMX_CLIENT in dmx-hid.ino and dmx-udmx.ino, \
        and only define IS_DMX_SERVER in dmx-server.ino."
    #else
        // Is client: include client DMX lib if SPLIT_MODE is set and real DMX lib if not
        #if SPLIT_MODE
            #include "dmxusb-dmx-lib-client.h"
        #else
            #include "dmxusb-dmx-lib-real.h"
        #endif
    #endif
#else
    #if IS_DMX_SERVER
        // Is server: include real DMX lib if SPLIT_MODE is set and throw an error if not
        #if SPLIT_MODE
            #include "dmxusb-dmx-lib-real.h"
        #else
            #error "Server isn't needed if split mode isn't enabled (SPLIT_MODE)."
        #endif
    #else
        #error "Sketch must be either client (IS_DMX_CLIENT) or server (IS_DMX_SERVER)."
    #endif
#endif
