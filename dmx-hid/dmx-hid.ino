/* Name: main.c
   Project: hid-data, example how to use HID for data transfer
   Author: Christian Starkjohann
   Creation Date: 2008-04-11
   Tabsize: 4
   Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
   License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
*/

#define IS_DMX_CLIENT 1

// ==============================================================================
// includes
// ------------------------------------------------------------------------------

// DMX library
#include <dmxusb-dmx-lib.h>
// HID USB library (includes config)
#include <dmxusb-usb-hid-lib.h>

// AVR Libc (see http://www.nongnu.org/avr-libc/)
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>

// debugging
#if DEBUG_ENABLED
#include <dmx-debug-lib.h>
#endif

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
PROGMEM const char usbHidReportDescriptor[22] = {    /* USB report descriptor */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop) // incorrect: "Vendor Defined Page 1"
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x21,                    //   REPORT_COUNT (33)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x92, 0x02, 0x01,              //   OUTPUT (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};
/* Since we define only one output report, we don't use report-IDs (which
   would be the first byte of the report). The entire report consists of 33
   opaque data bytes.
*/
#pragma GCC diagnostic pop

// ==============================================================================
// Globals
// ------------------------------------------------------------------------------
// usb-related globals
static u08 usb_state;
static u08 usb_length;
static u08 usb_index;
static u08 usb_mode;
static u16 cur_channel;

// LED keep alive counter
static u16 lka_count;

/* ------------------------------------------------------------------------- */

/* usbFunctionRead() is called when the host requests a chunk of data from
   the device. For more information see the documentation in usbdrv/usbdrv.h.
*/
uchar usbFunctionRead(uchar *data, uchar len)
{
  (void) data;
  (void) len;
  //TODO implement
  return 0xFF;
  /*
  if (usb_state != usb_GetChannelRange) {
    return 0xFF;  // stall if not in good state
  }
  lka_count = 0;

  // update data from stored channels
  len = min(len, end_channel - cur_channel);
  dmx_get_range(cur_channel, len, data);
  cur_channel += len;

#if DEBUG_ENABLED && DEBUG_USB
  dbg_print(F("read"));
  dbg_hexdump(data, len);
  dbg_print('\n');
#endif

  if (cur_channel >= end_channel)
    usb_state = usb_Idle;
  return len;
  */
}

/**
 * Returns 0 on success and 1 on failure.
 */
inline uint8_t handle_written_byte(uint8_t value) {
  switch(usb_state) {
    case usb_NotInitialized:
        // fall-through
    case usb_Idle:
#if DEBUG_ENABLED && DEBUG_PARSING
      dbg_print(F("write in Idle / NotInitialized\n"));
#endif
      // illegal states for a write
      // no need to set usb_state to usb_Idle:
      // if we are in usb_NotInitialized, we want to stay there
      // and if we are in usb_Idle, switching to usb_Idle isn't neccessary
      return 1;
    case usb_InIgnoredBytes:
      // ignore; do nothing
      break;

    case usb_SetReportStart:
#if DEBUG_ENABLED && DEBUG_PARSING
      dbg_print(F("Command: "));
      dbg_print(value);
      dbg_print('\n');
#endif
      if(value < 16) {
        cur_channel = value * BLOCK_SIZE;
        // -1 because the first byte is the command.
        if(cur_channel + usb_length - 1 > NUM_CHANNELS) {
#if DEBUG_ENABLED && DEBUG_PARSING
      dbg_print(F("Error: out of range\n"));
#endif
          // End channel out of range. Shouldn't happen in reality:
          // BLOCK_SIZE = 32, highest block index = 15 => highest start cur_channel = 32 * 15.
          // Report length 33; start byte is command => most data bytes in one report = 32.
          // => maximum required length = 32 * 16 = 512 = NUM_CHANNELS.
          usb_state = usb_Idle;
          return 1;
        }
        usb_state = usb_SetChannelRange;
      } else if(value == cmd_SetMode)
        usb_state = usb_SetMode_ExpectingMode;
      else if(value == cmd_SetTimings || value == cmd_StoreTimings)
        usb_state = usb_InIgnoredBytes;
      else {
        // unknown command
        usb_state = usb_Idle;
        return 1;
      }
      break;

    case usb_SetMode_ExpectingMode:
#if DEBUG_ENABLED && DEBUG_PARSING
      dbg_print(F("Mode: "));
      dbg_print(value);
      dbg_print('\n');
#endif
      if(value & ~mode_mask) {
        // illegal mode
        usb_state = usb_Idle;
        return 1;
      }
      usb_mode = value;
      usb_state = usb_InIgnoredBytes;
      break;

    case usb_SetChannelRange:
#if DEBUG_ENABLED && DEBUG_PARSING
      dbg_print(cur_channel);
      dbg_print(F(": "));
      dbg_print(value);
      dbg_print('\n');
#endif
      dmx_set_channel(cur_channel++, value);
      // No need to check if cur_channel is in range
      break;

    default:
#if DEBUG_ENABLED && DEBUG_PARSING
      dbg_print(F("Unknown state\n"));
#endif
      // unknown state
      usb_state = usb_Idle;
      return 1;
  }
  return 0;
}

/* usbFunctionWrite() is called when the host sends a chunk of data to the
   device. For more information see the documentation in usbdrv/usbdrv.h.
*/
uchar   usbFunctionWrite(uchar *data, uchar len)
{
#if DEBUG_ENABLED && DEBUG_USB
  dbg_print(F("write"));
  dbg_hexdump(data, len);
  dbg_print('\n');
#endif

  uint8_t failure = 0;
  
  usb_index += len;
  failure |= usb_index > usb_length;

  // optimization for SetChannelRange
  if(usb_state == usb_SetChannelRange && !failure) {
    dmx_set_range(cur_channel, len, data);
    cur_channel += len;
  }
  else
    for(uint8_t i = 0; i < len && !failure; i ++) {
      // optimization for SetChannelRange
      if(usb_state == usb_SetChannelRange && i < len - 1 && !failure) {
        uint8_t remaining = len - i;
        dmx_set_range(cur_channel + i, remaining, data);
        cur_channel += remaining;
        break;
      }

      failure |= handle_written_byte(data[i]);
    }

  if(failure)
    usb_state = usb_Idle;

  return failure ? 255 : (usb_index == usb_length ? 1 : 0);

/*
  if (usb_state != usb_SetChannelRange) {
    return 0xFF;  // stall if not in good state
  }
  lka_count = 0;

  // update channel values from received data
  len = min(len, end_channel - cur_channel);

  dmx_set_range(cur_channel, len, data);
  cur_channel += len;

  if (cur_channel >= end_channel) {
    usb_state = usb_Idle;
    return 1;   // tell driver we've got all data
  }
  return 0;   // otherwise, tell we want still more data
*/
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  // two explicit casts to disable warnings
  usbRequest_t *rq = (usbRequest_t *) (void *) data;

#if DEBUG_ENABLED && DEBUG_USB
  dbg_print(F("setup"));
  dbg_usbrequest(data);
  dbg_print('\n');
#endif
#if DEBUG_ENABLED && DEBUG_DMX_VALUES
  dbg_print(F("dmx vals"));
  dbg_hexdump(dmx_data, NUM_CHANNELS);
  dbg_print('\n');
#endif

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {  /* HID class request */
    if (rq->bRequest == USBRQ_HID_SET_REPORT) {
      /* since we have only one report type, we can ignore the report-ID */
#if DEBUG_ENABLED && DEBUG_USB
      dbg_print(F("SET_REPORT\n"));
#endif
      usb_index = 0;
      usb_length = rq->wLength.word;
      usb_state = usb_SetReportStart;
      return USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */
    } else {
#if DEBUG_ENABLED && DEBUG_USB
        dbg_print(F("not SET_REPORT\n"));
#endif
    }
  } else {
    /* ignore vendor type requests, we don't use any */
#if DEBUG_ENABLED && DEBUG_USB
    dbg_print(F("not HID class\n"));
#endif
  }
  return 0;
}

/* ------------------------------------------------------------------------- */

void hadAddressAssigned(void) {
  usb_state = usb_Idle;
  sbi(LED_GREEN_PORT, LED_GREEN_BIT);
  cbi(LED_YELLOW_PORT, LED_YELLOW_BIT);
}

void initForUsbConnectivity()
{
  uchar i = 0;

  /* enforce USB re-enumerate: */
  usbDeviceDisconnect();  /* do this while interrupts are disabled */
  while (--i) {       /* fake USB disconnect for > 250 ms */
    wdt_reset();
    _delay_ms(1);
  }
  usbDeviceConnect();
  usbInit();
}

void init() {
#if DEBUG_ENABLED
  Serial.begin(DEBUG_BAUD);
  dbg_print(F("Init..."));
#endif

  usb_state = usb_NotInitialized;
  lka_count = 0xffff;

  // clear Power On reset flag
  MCUSR &= ~(1 << PORF);

  // configure IO-Ports; most are unused, we set them to outputs to have defined voltages
  DDRB = 0xFF;    // unused
  DDRC = 0xFF;    // unused
  // unused except PD2 / INT0 (used by USB driver),
  // and PD1 (TX) which needs to be an output anyway.
  DDRD = (~(USBMASK) & ~(1 << 2)) & 0xFF;

  // LEDs are now outputs because all unused pins are outputs

  // welcome light
  cbi(LED_GREEN_PORT, LED_GREEN_BIT);
  sbi(LED_YELLOW_PORT, LED_YELLOW_BIT);

  wdt_enable(WDTO_1S);  // enable watchdog timer

  // init usb
  PORTB = 0;        // no pullups on USB pins
  initForUsbConnectivity(); // enumerate device

  dmx_init();

  sei();

#if DEBUG_ENABLED
  dbg_print(F(" complete!\n"));
#endif
}

int main()
{
  init();

  for(;;) {              /* main event loop */
    wdt_reset();
    usbPoll();
    dmx_poll();

    // keep yellow led on?
    if (lka_count < 0xfff ) {
      if (lka_count++ == 0)
        sbi(LED_YELLOW_PORT, LED_YELLOW_BIT);
    } else if (lka_count == 0xfff) {
      lka_count++;
      cbi(LED_YELLOW_PORT, LED_YELLOW_BIT);
    }
  }
  return 0;
}
