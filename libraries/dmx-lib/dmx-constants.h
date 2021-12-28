// Setting this will disable outputting any DMX data.
// If set, the interface will not change any registers for timer0 or the serial port,
// and will also not define any interrupt handlers.
// This makes the serial port free for debugging.
// Additionally, the dmx_data buffer is made globally visible (again, for debugging).
#define DISABLE_DMX_OUTPUT 0

// If enabled, the green LED will be toggled each time an unexpected TXC0 interrupt is generated.
// This happens if an UDRE0 interrupt couldn't be handled in time,
// which happens if the USB driver is active for too long.
// If this happens, the current DMX packet transmission to fail.
#define DEBUG_UNEXPECTED_TXC0 1
#define HANDLE_TXC0_VIA_INTERRUPT 1
#define HANDLE_UDRE0_VIA_INTERRUPT 1

#define NUM_CHANNELS 512		// number of channels in DMX-512

// values for dmx_state
#define dmx_Off 0
#define dmx_NewPacket 1	
#define dmx_InPacket 2
#define dmx_EndOfPacket 3
#define dmx_InBreak 4
#define dmx_InMAB 5

// LED constants
#define LED_GREEN_PORT PORTD
#define LED_GREEN_BIT 3
#define LED_YELLOW_PORT PORTD
#define LED_YELLOW_BIT 4
