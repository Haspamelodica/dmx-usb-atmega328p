#include <dmx-lib.h>

// For DEBUG_USB and DEBUG_DMX_VALUES to work, set DISABLE_DMX_OUTPUT in libraries/dmx-lib/dmx-constants.h.
// If enabled, debug messages will be output on the serial port.
#define DEBUG_USB 0
#define DEBUG_PARSING 0
#define DEBUG_DMX_VALUES 0
#define DEBUG_BAUD 1000000

// values for usb_state
#define usb_NotInitialized 0
#define usb_Idle 1
#define usb_InIgnoredBytes 2
#define usb_SetReportStart 3
#define usb_SetMode_ExpectingMode 4
#define usb_SetChannelRange 5

// derived values - don't change
#define DEBUG_ENABLED (DISABLE_DMX_OUTPUT && (DEBUG_USB || DEBUG_PARSING || DEBUG_DMX_VALUES))
