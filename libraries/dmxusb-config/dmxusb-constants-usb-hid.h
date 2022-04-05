#include "dmxusb-constants-usb.h"
#include "dmxusb-config-usb-hid.h"

#define DEBUG_ENABLED (DISABLE_DMX_OUTPUT && (DEBUG_USB || DEBUG_PARSING || DEBUG_DMX_VALUES))
#if !DISABLE_DMX_OUTPUT && (DEBUG_USB || DEBUG_PARSING || DEBUG_DMX_VALUES)
    #warning "DEBUG_USB, DEBUG_PARSING or DEBUG_DMX_VALUES is set, but debugging not enabled. DEBUG_USB, DEBUG_PARSING and DEBUG_DMX_VALUES will have no effect."
#endif

// values for usb_state
#define usb_NotInitialized 0
#define usb_Idle 1
#define usb_InIgnoredBytes 2
#define usb_SetReportStart 3
#define usb_SetMode_ExpectingMode 4
#define usb_SetChannelRange 5

#define BLOCK_SIZE 32

// Commands
// Commands 0-15 set blocks of output data.
// Set interface mode. For modes see below.
#define cmd_SetMode 16
// Set various timing parameters. Ignored.
#define cmd_SetTimings 17
// Store timings to internal memory, probably? Ignored.
#define cmd_StoreTimings 18

// Modes
#define mode_in_to_out_bit (1 << 0)
#define mode_pc_to_out_bit (1 << 1)
#define mode_in_to_pc_bit (1 << 2)
#define mode_mask (mode_in_to_out_bit | mode_pc_to_out_bit | mode_in_to_pc_bit)
