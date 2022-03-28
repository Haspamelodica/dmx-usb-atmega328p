#include "dmxusb-constants-usb.h"
#include "dmxusb-config-usb-udmx.h"

//TODO throw an error instead
#define DEBUG_ENABLED (DISABLE_DMX_OUTPUT && (DEBUG_USB || DEBUG_DMX_VALUES))

// values for usb_state
#define usb_NotInitialized 0
#define usb_Idle 1
#define usb_ChannelRange 2

/*
 *  udmx-cmds.h
 *  Created by Max Egger on 14.02.06.
 */
#define cmd_SetSingleChannel 1
/* usb request for cmd_SetSingleChannel:
    bmRequestType:  ignored by device, should be USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT
    bRequest:       cmd_SetSingleChannel
    wValue:         value of channel to set [0 .. 255]
    wIndex:         channel index to set [0 .. 511]
    wLength:        ignored
*/
#define cmd_SetChannelRange 2
/* usb request for cmd_SetChannelRange:
    bmRequestType:  ignored by device, should be USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT
    bRequest:       cmd_SetChannelRange
    wValue:         number of channels to set [1 .. 512-wIndex]
    wIndex:         index of first channel to set [0 .. 511]
    wLength:        length of data, must be >= wValue
*/
#define cmd_StartBootloader 0xf8
// Start Bootloader for Software updates

#define err_BadChannel 1
#define err_BadValue 2
