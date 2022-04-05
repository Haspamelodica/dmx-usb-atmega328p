#include "dmxusb-constants-common.h"
#include "dmxusb-config-dmx.h"

#if DEBUG_UNEXPECTED_TXC0 && !HANDLE_TXC0_VIA_INTERRUPT
    #warning "DEBUG_UNEXPECTED_TXC0 is set, but HANDLE_TXC0_VIA_INTERRUPT is not enabled. DEBUG_UNEXPECTED_TXC0 will have no effect."
#endif

#if ATOMIC_UDRE && !HANDLE_UDRE0_VIA_INTERRUPT
    #warning "ATOMIC_UDRE is set, but HANDLE_UDRE0_VIA_INTERRUPT is not enabled. ATOMIC_UDRE will have no effect."
#endif

// for real dmxlib
#define dmx_Off 0
#define dmx_NewPacket 1
#define dmx_InMAB 2
#define dmx_InBreak 3
#define dmx_InPacket 4
#define dmx_EndOfPacket 5

// for server and client
// Avoid "common" values like 0 or 0xFF as sync bytes;
// those are very probable to occur in DMX data
#define SYNC_BYTE_1 42
#define SYNC_BYTE_2 120

// for client
#define client_SyncByte1 0
#define client_SyncByte2 1
#define client_Data 2

// for server
#define server_WaitingSyncByte1 0
#define server_WaitingSyncByte2 1
#define server_Data 2
