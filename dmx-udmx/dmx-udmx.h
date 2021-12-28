// For DEBUG_USB and DEBUG_DMX_VALUES to work, set DISABLE_DMX_OUTPUT in dmx-lib/dmx-constants.h.
// If enabled, debug messages will be output on the serial port.
#define DEBUG_USB 0
#define DEBUG_DMX_VALUES 0
#define DEBUG_BAUD 230400

// values for usb_state
#define usb_NotInitialized 0
#define usb_Idle 1
#define usb_ChannelRange 2
