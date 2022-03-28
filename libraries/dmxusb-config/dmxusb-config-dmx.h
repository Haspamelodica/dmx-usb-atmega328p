// Settings in this file only have an effect for the Atmega the real DMX interface is running on.

// Setting this will disable outputting any DMX data.
// If set, the interface will not change any registers for timer0 or the serial port,
// and will also not define any interrupt handlers.
// This makes the serial port free for debugging.
// Additionally, the dmx_data buffer is made globally visible (again, for debugging).
#define DISABLE_DMX_OUTPUT 0

// If enabled, the green LED will be toggled each time an unexpected TXC0 interrupt is generated.
// This happens if an UDRE0 interrupt couldn't be handled in time,
// which happens if the USB driver is active for too long.
// If this happens, the current DMX packet transmission will fail.
//TODO this probably only works if HANDLE_TXC0_VIA_INTERRUPT?
#define DEBUG_UNEXPECTED_TXC0 0

#define HANDLE_TXC0_VIA_INTERRUPT 1
#define HANDLE_UDRE0_VIA_INTERRUPT 1

// VUSB requires interrupts to not be disabled for longer than 25 cycles.
// Because of this, the interrupt handler for UDRE has to enable interrupts in its body.
// However, this increases the likelihood of packet transmissions failing,
// which in turn makes DMX output very choppy and unreliable.
// So, this behaviour (enabling interrupts in the UDRE handler) can be disabled
// by enabling ATOMIC_UDRE.
//TODO this probably only works if HANDLE_UDRE0_VIA_INTERRUPT?
#define ATOMIC_UDRE 0
