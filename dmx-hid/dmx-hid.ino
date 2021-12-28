/* Name: main.c
   Project: hid-data, example how to use HID for data transfer
   Author: Christian Starkjohann
   Creation Date: 2008-04-11
   Tabsize: 4
   Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
   License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
*/

/*
  This example should run on most AVRs with only little changes. No special
  hardware resources except INT0 are used. You may have to change usbconfig.h for
  different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
  at least be connected to INT0 as well.
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>

#include "dmx-hid-lib.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
PROGMEM const char usbHidReportDescriptor[22] = {    /* USB report descriptor */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop) //incorrect: "Vendor Defined Page 1"
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x80,                    //   REPORT_COUNT (128)
    0x09, 0x00,                    //   USAGE (Undefined)
    0x92, 0x02, 0x01,              //   OUTPUT (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};
/* Since we define only one feature report, we don't use report-IDs (which
   would be the first byte of the report). The entire report consists of 128
   opaque data bytes.
*/
#pragma GCC diagnostic pop

const char hexdigit_to_char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void hexdump(void *data, size_t byte_len) {
  uint8_t *data_uint8 = (uint8_t *) data;
  for (size_t i = 0; i < byte_len; i ++) {
    Serial.print(' ');
    Serial.print(hexdigit_to_char[data_uint8[i] >> 4]);
    Serial.print(hexdigit_to_char[data_uint8[i] & 0xF]);
  }
}

/* The following variables store the status of the current data transfer */
static uchar    currentAddress;
static uchar    bytesRemaining;

/* ------------------------------------------------------------------------- */

/* usbFunctionRead() is called when the host requests a chunk of data from
   the device. For more information see the documentation in usbdrv/usbdrv.h.
*/
uchar   usbFunctionRead(uchar *data, uchar len)
{
  Serial.print("read ");
  hexdump(data, len);
  Serial.print('\n');
  if (len > bytesRemaining)
    len = bytesRemaining;
  //eeprom_read_block(data, (uchar *)0 + currentAddress, len);
  currentAddress += len;
  bytesRemaining -= len;
  return len;
}

/* usbFunctionWrite() is called when the host sends a chunk of data to the
   device. For more information see the documentation in usbdrv/usbdrv.h.
*/
uchar   usbFunctionWrite(uchar *data, uchar len)
{
  Serial.print("write");
  hexdump(data, len);
  Serial.print('\n');
  if (bytesRemaining == 0)
    return 1;               /* end of transfer */
  if (len > bytesRemaining)
    len = bytesRemaining;
  //eeprom_write_block(data, (uchar *)0 + currentAddress, len);
  currentAddress += len;
  bytesRemaining -= len;
  return bytesRemaining == 0 ? 1 : 0; /* return 1 if this was the last chunk */
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  //two explicit casts to disable warnings
  usbRequest_t *rq = (usbRequest_t *) (void *) data;

  Serial.print("setup");
  hexdump(data, 8);
  Serial.print(". req type ");
  Serial.print(rq->bmRequestType);
  Serial.print(", req ");
  Serial.print(rq->bRequest);
  Serial.print(", value ");
  Serial.print(rq->wValue.word);
  Serial.print(", wIndex ");
  Serial.print(rq->wIndex.word);
  Serial.print(", length ");
  Serial.print(rq->wLength.word);
  Serial.print('\n');

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {  /* HID class request */
    if (rq->bRequest == USBRQ_HID_GET_REPORT) { /* wValue: ReportType (highbyte), ReportID (lowbyte) */
      /* since we have only one report type, we can ignore the report-ID */
	  Serial.println("GET_REPORT");
      bytesRemaining = rq->wLength.word;
      currentAddress = 0;
      return USB_NO_MSG;  /* use usbFunctionRead() to obtain data */
    } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
      /* since we have only one report type, we can ignore the report-ID */
	  Serial.println("SET_REPORT");
      bytesRemaining = rq->wLength.word;
      currentAddress = 0;
      return bytesRemaining == 0 ? 0 : USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */
    } else {
	    Serial.println("neither GET_REPORT nor SET_REPORT");
	}
  } else {
    /* ignore vendor type requests, we don't use any */
    Serial.println("not HID class");
  }
  return 0;
}

/* ------------------------------------------------------------------------- */

int main()
{
  Serial.begin(230400);
  Serial.print("Init...");
  uchar   i;

  usbInit();
  usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
  i = 0;
  while (--i) {           /* fake USB disconnect for > 250 ms */
    _delay_ms(1);
  }
  usbDeviceConnect();
  sei();

  Serial.print(" complete!\n");

  for (;;) {              /* main event loop */
    usbPoll();
  }
  return 0;
}

/* ------------------------------------------------------------------------- */
