
// ==============================================================================
// uDMX.c
// firmware for usb to dmx interface
//
// License:
// The project is built with AVR USB driver by Objective Development, which is
// published under an own licence based on the GNU General Public License (GPL).
// usb2dmx is also distributed under this enhanced licence. See Documentation.
//
// target-cpu: ATMega8 @ 12MHz
// created 2006-02-09 mexx
//
// version 1.4     2009-06-09 me@anyma.ch
//    - changed usb init routine
// version 1.3:    2008-11-04 me@anyma.ch
// ==============================================================================


// ==============================================================================
// includes
// ------------------------------------------------------------------------------

// UDMX USB library
#include <dmx-udmx-lib.h>

// AVR Libc (see http://www.nongnu.org/avr-libc/)
#include <avr/io.h>     // include I/O definitions (port names, pin names, etc)
#include <avr/pgmspace.h> // include program space (for PROGMEM)
#include <avr/interrupt.h>  // include interrupt support
#include <avr/wdt.h>    // include watchdog timer support
#include <avr/sleep.h>    // include cpu sleep support
#include <util/delay.h>

// local includes
#include "dmx-udmx.h"
#include "uDMX_cmds.h"    // USB command and error constants

typedef unsigned char  u08;
typedef   signed char  s08;
typedef unsigned short u16;
typedef   signed short s16;


typedef struct _midi_msg {
  u08 cn : 4;
  u08 cin : 4;
  u08 byte[3];
} midi_msg;

// ==============================================================================
// Constants
// ------------------------------------------------------------------------------

// device serial number, formatted as YearMonthDayNCounter
PROGMEM const int usbDescriptorStringSerialNumber[] = {USB_STRING_DESCRIPTOR_HEADER(11), '1', '1', '0', '4', '2', '8', 'N', '0', '0', '7', '1'};


// ==============================================================================
// Globals
// ------------------------------------------------------------------------------
// dmx-related globals
static u08 dmx_data[NUM_CHANNELS];
static u16 out_idx;     // index of next frame to send
static u16 packet_len = 0;  // we only send frames up to the highest channel set
static u08 dmx_state = dmx_Off;

// usb-related globals
static u08 usb_state;
static u16 cur_channel, end_channel;
static u08 reply[8];

//led keep alive counter
static u16 lka_count;

// This descriptor is based on http://www.usb.org/developers/devclass_docs/midi10.pdf
// u
// Appendix B. Example: Simple MIDI Adapter (Informative)
// B.1 Device Descriptor
//
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
static PROGMEM const uchar deviceDescrMIDI[] = { /* USB device descriptor */
  18,     /* sizeof(usbDescriptorDevice): length of descriptor in bytes */
  USBDESCR_DEVICE,  /* descriptor type */
  0x10, 0x01,   /* USB version supported */
  0,      /* device class: defined at interface level */
  0,      /* subclass */
  0,      /* protocol */
  8,      /* max packet size */
  USB_CFG_VENDOR_ID,  /* 2 bytes */
  USB_CFG_DEVICE_ID,  /* 2 bytes */
  USB_CFG_DEVICE_VERSION, /* 2 bytes */
  1,      /* manufacturer string index */
  2,      /* product string index */
  0,      /* serial number string index */
  1,      /* number of configurations */
};

// B.2 Configuration Descriptor
static PROGMEM const uchar configDescrMIDI[] = { /* USB configuration descriptor */
  9,      /* sizeof(usbDescrConfig): length of descriptor in bytes */
  USBDESCR_CONFIG,  /* descriptor type */
  101, 0,     /* total length of data returned (including inlined descriptors) */
  2,      /* number of interfaces in this configuration */
  1,      /* index of this configuration */
  0,      /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
  (1 << 7) | USBATTR_SELFPOWER, /* attributes */
#else
  (1 << 7), /* attributes */
#endif
  USB_CFG_MAX_BUS_POWER / 2,  /* max USB current in 2mA units */

  // B.3 AudioControl Interface Descriptors
  // The AudioControl interface describes the device structure (audio function topology)
  // and is used to manipulate the Audio Controls. This device has no audio function
  // incorporated. However, the AudioControl interface is mandatory and therefore both
  // the standard AC interface descriptor and the classspecific AC interface descriptor
  // must be present. The class-specific AC interface descriptor only contains the header
  // descriptor.

  // B.3.1 Standard AC Interface Descriptor
  // The AudioControl interface has no dedicated endpoints associated with it. It uses the
  // default pipe (endpoint 0) for all communication purposes. Class-specific AudioControl
  // Requests are sent using the default pipe. There is no Status Interrupt endpoint provided.
  /* AC interface descriptor follows inline: */
  9,      /* sizeof(usbDescrInterface): length of descriptor in bytes */
  USBDESCR_INTERFACE, /* descriptor type */
  0,      /* index of this interface */
  0,      /* alternate setting for this interface */
  0,      /* endpoints excl 0: number of endpoint descriptors to follow */
  1,      /* */
  1,      /* */
  0,      /* */
  0,      /* string index for interface */

  // B.3.2 Class-specific AC Interface Descriptor
  // The Class-specific AC interface descriptor is always headed by a Header descriptor
  // that contains general information about the AudioControl interface. It contains all
  // the pointers needed to describe the Audio Interface Collection, associated with the
  // described audio function. Only the Header descriptor is present in this device
  // because it does not contain any audio functionality as such.
  /* AC Class-Specific descriptor */
  9,      /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
  36,     /* descriptor type */
  1,      /* header functional descriptor */
  0x0, 0x01,    /* bcdADC */
  9, 0,     /* wTotalLength */
  1,      /* */
  1,      /* */

  // B.4 MIDIStreaming Interface Descriptors

  // B.4.1 Standard MS Interface Descriptor
  /* interface descriptor follows inline: */
  9,      /* length of descriptor in bytes */
  USBDESCR_INTERFACE, /* descriptor type */
  1,      /* index of this interface */
  0,      /* alternate setting for this interface */
  2,      /* endpoints excl 0: number of endpoint descriptors to follow */
  1,      /* AUDIO */
  3,      /* MS */
  0,      /* unused */
  0,      /* string index for interface */

  // B.4.2 Class-specific MS Interface Descriptor
  /* MS Class-Specific descriptor */
  7,      /* length of descriptor in bytes */
  36,     /* descriptor type */
  1,      /* header functional descriptor */
  0x0, 0x01,    /* bcdADC */
  65, 0,      /* wTotalLength */

  // B.4.3 MIDI IN Jack Descriptor
  6,      /* bLength */
  36,     /* descriptor type */
  2,      /* MIDI_IN_JACK desc subtype */
  1,      /* EMBEDDED bJackType */
  1,      /* bJackID */
  0,      /* iJack */

  6,      /* bLength */
  36,     /* descriptor type */
  2,      /* MIDI_IN_JACK desc subtype */
  2,      /* EXTERNAL bJackType */
  2,      /* bJackID */
  0,      /* iJack */

  //B.4.4 MIDI OUT Jack Descriptor
  9,      /* length of descriptor in bytes */
  36,     /* descriptor type */
  3,      /* MIDI_OUT_JACK descriptor */
  1,      /* EMBEDDED bJackType */
  3,      /* bJackID */
  1,      /* No of input pins */
  2,      /* BaSourceID */
  1,      /* BaSourcePin */
  0,      /* iJack */
  9,      /* bLength of descriptor in bytes */
  36,     /* bDescriptorType */
  3,      /* MIDI_OUT_JACK bDescriptorSubtype */
  2,      /* EXTERNAL bJackType */
  4,      /* bJackID */
  1,      /* bNrInputPins */
  1,      /* baSourceID (0) */
  1,      /* baSourcePin (0) */
  0,      /* iJack */


  // B.5 Bulk OUT Endpoint Descriptors

  //B.5.1 Standard Bulk OUT Endpoint Descriptor
  9,      /* bLenght */
  USBDESCR_ENDPOINT,  /* bDescriptorType = endpoint */
  0x1,      /* bEndpointAddress OUT endpoint number 1 */
  3,      /* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
  8, 0,     /* wMaxPacketSize */
  10,     /* bIntervall in ms */
  0,      /* bRefresh */
  0,      /* bSyncAddress */

  // B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
  5,      /* bLength of descriptor in bytes */
  37,     /* bDescriptorType */
  1,      /* bDescriptorSubtype */
  1,      /* bNumEmbMIDIJack  */
  1,      /* baAssocJackID (0) */


  //B.6 Bulk IN Endpoint Descriptors

  //B.6.1 Standard Bulk IN Endpoint Descriptor
  9,      /* bLenght */
  USBDESCR_ENDPOINT,  /* bDescriptorType = endpoint */
  0x81,     /* bEndpointAddress IN endpoint number 1 */
  3,      /* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
  8, 0,     /* wMaxPacketSize */
  10,     /* bIntervall in ms */
  0,      /* bRefresh */
  0,      /* bSyncAddress */

  // B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
  5,      /* bLength of descriptor in bytes */
  37,     /* bDescriptorType */
  1,      /* bDescriptorSubtype */
  1,      /* bNumEmbMIDIJack (0) */
  3,      /* baAssocJackID (0) */
};
#pragma GCC diagnostic pop

const char hexdigit_to_char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#if (DEBUG_USB | DEBUG_DMX_VALUES)
void hexdump(void *data, size_t byte_len) {
  uint8_t *data_uint8 = (uint8_t *) data;
  for (size_t i = 0; i < byte_len; i ++) {
    Serial.print(' ');
    Serial.print(hexdigit_to_char[data_uint8[i] >> 4]);
    Serial.print(hexdigit_to_char[data_uint8[i] & 0xF]);
  }
}
#endif

// ==============================================================================
// - sleepIfIdle
// ------------------------------------------------------------------------------
void sleepIfIdle()
{
#if ENABLE_SLEEP_IF_IDLE
  if (TIFR1 & BV(TOV1)) {
    cli();
    if (!(EIFR & BV(INTF1))) {
      // no activity on INT1 pin for >3ms => suspend:

      // turn off leds
      cbi(LED_GREEN_PORT, LED_GREEN_BIT);
      cbi(LED_YELLOW_PORT, LED_YELLOW_BIT);

      // - reconfigure INT1 to level-triggered and enable for wake-up
      cbi(EICRA, ISC10);
      sbi(EIMSK, INT1);
      // - go to sleep
      wdt_disable();
      sleep_enable();
      sei();
      sleep_cpu();

      // wake up
      sleep_disable();
      // - reconfigure INT1 to any edge for SE0-detection
      cbi(EIMSK, INT1);
      sbi(EICRA, ISC10);
      // - re-enable watchdog
      wdt_reset();
      wdt_enable(WDTO_1S);
    }
    sei();
    // clear INT1 flag
    sbi(EIFR, INTF1);
    // reload timer and clear overflow
    TCCR1B = 1;
    TCNT1 = 25000;    // max ca. 3ms between SE0
    sbi(TIFR1, TOV1);

    //welcome light again
    cbi(LED_GREEN_PORT, LED_GREEN_BIT);
    sbi(LED_YELLOW_PORT, LED_YELLOW_BIT);
  }
#endif
}

void hadAddressAssigned(void) {
  usb_state = usb_Idle;
  sbi(LED_GREEN_PORT, LED_GREEN_BIT);
  cbi(LED_YELLOW_PORT, LED_YELLOW_BIT);
}

#if ENABLE_SLEEP_IF_IDLE
// ------------------------------------------------------------------------------
// - INT1_vec (dummy for wake-up)
// ------------------------------------------------------------------------------
ISR(INT1_vect) {}
#endif

// ------------------------------------------------------------------------------
// - Enumerate device
// ------------------------------------------------------------------------------

static void initForUsbConnectivity(void)
{
  uchar   i = 0;

  /* enforce USB re-enumerate: */
  usbDeviceDisconnect();  /* do this while interrupts are disabled */
  while (--i) {       /* fake USB disconnect for > 250 ms */
    wdt_reset();
    _delay_ms(1);
  }
  usbDeviceConnect();
  usbInit();
}

// ==============================================================================
// - init
// ------------------------------------------------------------------------------
void init(void)
{
  dmx_state = dmx_Off;
  lka_count = 0xffff;

  //clear Power On reset flag
  MCUSR &= ~(1 << PORF);

  // configure IO-Ports; most are unused, we set them to outputs to have defined voltages
  DDRB = 0xFF;    // unused
  DDRC = 0xFF;    // unused
  // unused except PD2 / INT0 (used by USB driver),
  // maybe PD3 / INT1 if ENABLE_SLEEP_IF_IDLE (for bus activity detection (sleep)),
  // and PD1 (TX) which needs to be an output anyway.
  DDRD = (~(USBMASK) & ~(1 << 2)
#if ENABLE_SLEEP_IF_IDLE
         & ~(1 << 3)
#endif
         ) & 0xFF;

  //LEDs are now outputs because all unused pins are outputs

  //welcome light
  cbi(LED_GREEN_PORT, LED_GREEN_BIT);
  sbi(LED_YELLOW_PORT, LED_YELLOW_BIT);

  // init timer0 for DMX timing
  TCCR0B = 2; // prescaler 8 => 1 clock is 2/3 us for 12Mhz / .05 us for 16MHz (8 / F_CPU seconds)

#if ENABLE_SLEEP_IF_IDLE
  // init Timer 1  and Interrupt 1 for usb activity detection:
  // - set INT1 to any edge (polled by sleepIfIdle())
  cbi(EICRA, ISC11);
  sbi(EICRA, ISC10);
#endif

  wdt_enable(WDTO_1S);  // enable watchdog timer


  // set sleep mode to full power-down for minimal consumption
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

#if ENABLE_SLEEP_IF_IDLE
  // - set Timer 1 prescaler to 64 and restart timer
  TCCR1B = 3;
  TCNT1 = 0;
  sbi(TIFR1, TOV1);
#endif

  // init usb
  PORTB = 0;        // no pullups on USB pins
  initForUsbConnectivity(); // enumerate device

  sei();

  // init uart
  UBRR0L = F_CPU / (250000 * 16) - 1; UBRR0H =  0; // baud rate 250kbps
  UCSR0A = 0; // clear error flags
  UCSR0C = BV(USBS0) | BV(UCSZ01) | BV(UCSZ00); // 8 data bits, 2 stop bits, no parity (8N2)
  // don't turn on UART yet; and don't set UDRIE0 (TX Data Register Empty Interrupt Enable) yet, even if HANDLE_UDRE0_VIA_INTERRUPT.
  UCSR0B = 0
#if HANDLE_TXC0_VIA_INTERRUPT
  // Set TXCIE0 (TX Complete Interrupt Enable).
  | BV(TXCIE0)
#endif
  ;

  //  Serial.begin(230400);
  //  Serial.print(F("Init complete\n"));
}

#if HANDLE_TXC0_VIA_INTERRUPT
#if DEBUG_UNEXPECTED_TXC0
uint8_t txc0_expected;
#endif
uint8_t txc0_from_interrupt;
//Transmit Complete interrupt vector

ISR(USART_TX_vect) {
#if DEBUG_UNEXPECTED_TXC0
  if (!txc0_expected)
    LED_GREEN_PORT ^= BV(LED_GREEN_BIT);
#endif
  txc0_from_interrupt = 1;
}
#endif //HANDLE_TXC0_VIA_INTERRUPT

//Returns 1 if state is now end-of-packet
inline uint8_t handle_udre_inPacket() {
  // send next byte of dmx packet
  if (out_idx < packet_len) {
    UDR0 = dmx_data[out_idx++];
    return 0;
  }
  else {
#if HANDLE_UDRE0_VIA_INTERRUPT
	cbi(UCSR0B, UDRIE0); //disable UDRE (TX Data Register Empty) interrupt
#endif
#if (HANDLE_TXC0_VIA_INTERRUPT && DEBUG_UNEXPECTED_TXC0)
    txc0_expected = 1;
#endif
    dmx_state = dmx_EndOfPacket;
    return 1;
  }
}

#if HANDLE_UDRE0_VIA_INTERRUPT
ISR(USART_UDRE_vect) {
  /* Disabled because it makes things worse.
  //temporarily disable UDRIE0, but enable interrupts:
  //we want to be able to be interrupted from the UDRE interrupt handler as USB should take precedence,
  //but not cause a recursive loop
  cbi(UCSR0B, UDRIE0);
  sei();
  */
  
  while(UCSR0A & BV(UDRE0))
    if(dmx_state == dmx_InPacket)
	  if(handle_udre_inPacket())
		//return, not break, to skip re-enabling UDRIE0 if we are now in EndOfPacket
		return;

  /* Disabled because it makes things worse.
  //re-enable UDRIE0, but first disable interrupts:
  //we only want to handle the next UDRE after returning
  //cli();
  //sbi(UCSR0B, UDRIE0);
  */
}
#endif //HANDLE_UDRE0_VIA_INTERRUPT

// ------------------------------------------------------------------------------
// - Start Bootloader
// ------------------------------------------------------------------------------
// dummy function doing the jump to bootloader section (Adress 0xC00 on Atmega8; address 0x3F00 on Arduino Uno)
void (*jump_to_bootloader)(void) = (void (*)(void)) 0x3F00; __attribute__ ((unused))

void startBootloader(void) {


  MCUSR &= ~(1 << PORF);      // clear power on reset flag
  // this will hint the bootloader that it was forced

  cli();              // turn off interrupts
  wdt_disable();          // disable watchdog timer
  usbDeviceDisconnect();      // disconnect udmx from USB bus


  cbi(LED_GREEN_PORT, LED_GREEN_BIT);
  cbi(LED_YELLOW_PORT, LED_YELLOW_BIT);

  jump_to_bootloader();
}

uchar usbFunctionDescriptor(usbRequest_t * rq)
{
#if DEBUG_USB
  Serial.print(F("desc\n"));
#endif
  if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
    usbMsgPtr = (usbMsgPtr_t) deviceDescrMIDI;
    return sizeof(deviceDescrMIDI);
  } else {    /* must be config descriptor */
    usbMsgPtr = (usbMsgPtr_t) configDescrMIDI;
    return sizeof(configDescrMIDI);
  }
}

// ==============================================================================
// - usbFunctionSetup
// ------------------------------------------------------------------------------
uchar usbFunctionSetup(uchar data[8])
{
#if DEBUG_USB
  Serial.print(F("setup"));
  hexdump(data, 8);
  Serial.print('\n');
#endif
#if DEBUG_DMX_VALUES
  Serial.print(F("dmx vals"));
  hexdump(dmx_data, NUM_CHANNELS);
  Serial.print('\n');
#endif
  usbMsgPtr = (usbMsgPtr_t) reply;
  reply[0] = 0;
  if (data[1] == cmd_SetSingleChannel) {
    lka_count = 0;
    // get channel index from data.wIndex and check if in legal range [0..511]
    u16 channel = data[4] | (data[5] << 8);
    if (channel > 511) {
      reply[0] = err_BadChannel;
      return 1;
    }
    // get channel value from data.wValue and check if in legal range [0..255]
    if (data[3]) {
      reply[0] = err_BadValue;
      return 1;
    }
    dmx_data[channel] = data[2];
    // update dmx state
    if (channel >= packet_len) packet_len = channel + 1;
    if (dmx_state == dmx_Off) dmx_state = dmx_NewPacket;
  }
  else if (data[1] == cmd_SetChannelRange) {
    lka_count = 0;
    // get start and end channel index
    cur_channel = data[4] | (data[5] << 8);
    end_channel = cur_channel + (data[2] | (data[3] << 8));
    // check for legal channel range
    if ((end_channel - cur_channel) > (unsigned short) (data[6] | (data[7] << 8)))
    {
      reply[0] = err_BadValue;
      cur_channel = end_channel = 0;
      return 1;
    }
    if ((cur_channel > 511) || (end_channel > 512))
    {
      reply[0] = err_BadChannel;
      cur_channel = end_channel = 0;
      return 1;
    }
    // update usb state and wait for channel data
    usb_state = usb_ChannelRange;
    return 0xFF;

  } else if (data[1] == cmd_StartBootloader) {

    startBootloader();
  }

  return 0;
}

/*---------------------------------------------------------------------------*/
/* usbFunctionRead                                                           */
/*---------------------------------------------------------------------------*/

uchar usbFunctionRead(uchar * data, uchar)
{
#if DEBUG_USB
  Serial.print(F("read\n"));
#endif
  // DEBUG LED
  //PORTC ^= 0x02;

  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  data[3] = 0;
  data[4] = 0;
  data[5] = 0;
  data[6] = 0;

  return 7;
}

/*---------------------------------------------------------------------------*/
/* usbFunctionWriteOut                                                       */
/*                                                                           */
/* this Function is called if a MIDI Out message (from PC) arrives.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

void usbFunctionWriteOut(uchar * data, uchar len)
{
#if DEBUG_USB
  Serial.print(F("writeOut"));
  hexdump(data, len);
  Serial.print('\n');
#endif
  while (len >= sizeof(midi_msg)) {
    midi_msg* msg = (midi_msg*)data;

    switch (msg->byte[0]) {
      case 0xB0: {        // control change
          u08 chan_no = msg->byte[1] - 1;
          if (chan_no < 120) {  // controllers 120..127 are reserved for channel mode msg
            dmx_data[chan_no] = msg->byte[2] << 1;
            if (chan_no > packet_len) packet_len = chan_no;
          }
          if (dmx_state == dmx_Off) dmx_state = dmx_NewPacket;
          break;
        }
      case 0x90: {        // note on
          u08 chan_no = msg->byte[1] - 1;
          dmx_data[chan_no] = msg->byte[2] << 1;
          if (chan_no > packet_len) packet_len = chan_no;
          if (dmx_state == dmx_Off) dmx_state = dmx_NewPacket;
          break;
        }
      case 0x80: {        // note off
          u08 chan_no = msg->byte[1] - 1;
          dmx_data[chan_no] = 0;
          break;
        }
      default: break;
    }
    data += sizeof(midi_msg);
    len -= sizeof(midi_msg);
  }
}

// ------------------------------------------------------------------------------
// - usbFunctionWrite
// ------------------------------------------------------------------------------
uchar usbFunctionWrite(uchar* data, uchar len)
{
  //LED_GREEN_PORT ^= BV(LED_GREEN_BIT);
#if DEBUG_USB
  Serial.print(F("write"));
  hexdump(data, len);
  Serial.print('\n');
#endif
  if (usb_state != usb_ChannelRange) {
    return 0xFF;  // stall if not in good state
  }
  lka_count = 0;

  // update channel values from received data
  //uchar* data_end = data + len;
  //for (; (data < data_end) && (cur_channel < end_channel); ++data, ++cur_channel) {
  //  dmx_data[cur_channel] = *data;
  //}
  len = min(len, end_channel - cur_channel);
  memcpy(dmx_data + cur_channel, data, len);
  cur_channel += len;

  // update state
  if (cur_channel > packet_len) packet_len = cur_channel;
  if (dmx_state == dmx_Off) dmx_state = dmx_NewPacket;
  if (cur_channel >= end_channel) {
    usb_state = usb_Idle;
    return 1;   // tell driver we've got all data
  }
  return 0;   // otherwise, tell we want still more data
}

void pollDMX() {
  // do dmx transmission
  switch (dmx_state) {
    case dmx_NewPacket: {
        // start a new dmx packet:
        sbi(UCSR0B, TXEN0);  // enable UART transmitter
        out_idx = 0;    // reset output channel index
        sbi(UCSR0A, TXC0);  // reset Transmit Complete flag
        UDR0 =  0;    // send start byte
        dmx_state = dmx_InPacket;
#if HANDLE_TXC0_VIA_INTERRUPT
#if DEBUG_UNEXPECTED_TXC0
        txc0_expected = 0;
#endif
        txc0_from_interrupt = 0;
#endif //HANDLE_TXC0_VIA_INTERRUPT
#if HANDLE_UDRE0_VIA_INTERRUPT
		sbi(UCSR0B, UDRIE0); //enable UDRE (TX Data Register Empty) interrupt
#endif //HANDLE_UDRE0_VIA_INTERRUPT
        break;
      }
#if (!HANDLE_UDRE0_VIA_INTERRUPT)
    case dmx_InPacket: {
        if (!(UCSR0A & BV(UDRE0)))
          break;
        if (!handle_udre_inPacket())
          break;
		break;
      }
#endif
    case dmx_EndOfPacket: {
        if (
#if HANDLE_TXC0_VIA_INTERRUPT
			txc0_from_interrupt
#else
			UCSR0A & BV(TXC0)
#endif
		) {
#if HANDLE_TXC0_VIA_INTERRUPT
#if DEBUG_UNEXPECTED_TXC0
          txc0_expected = 0;
#endif
          txc0_from_interrupt = 0;
#endif //HANDLE_TXC0_VIA_INTERRUPT
          // send a BREAK:
          cbi(UCSR0B, TXEN0); // disable UART transmitter
          cbi(PORTD, 1);    // pull TX pin low

          sbi(GTCCR, PSRSYNC);  // reset timer prescaler
          //TCNT0 = 123;    // 132 clks = 88us //only correct for 12Mhz
          TCNT0 = F_CPU / 8 * 92 / 1000000; //prescaler is 8 and we want 92us (instead of 88us in the original)
          sbi(TIFR0, TOV0); // clear timer overflow flag
          dmx_state = dmx_InBreak;
        }
        break;
      }
    case dmx_InBreak: {
        if (TIFR0 & BV(TOV0)) {
          sleepIfIdle();  // if there's been no activity on USB for > 3ms, put CPU to sleep

          // end of BREAK: send MARK AFTER BREAK
          sbi(PORTD, 1);    // pull TX pin high
          sbi(GTCCR, PSRSYNC);  // reset timer prescaler
          //TCNT0 = 243;    // 12 clks = 8us //only correct for 12Mhz
          TCNT0 = F_CPU / 8 * 12 / 1000000; //prescaler is 8 and we want 12us (instead of 8us in the original)
          sbi(TIFR0, TOV0); // clear timer overflow flag
          dmx_state = dmx_InMAB;
        }
        break;
      }
    case dmx_InMAB: {
        if (TIFR0 & BV(TOV0)) {
          // end of MARK AFTER BREAK; start new dmx packet
          dmx_state = dmx_NewPacket;
        }
        break;
      }
  }
}

// ==============================================================================
// - main
// ------------------------------------------------------------------------------
int main(void)
{
  init();
  while (1) {

    // usb-related stuff
    wdt_reset();
    usbPoll();

    pollDMX();

    if (!packet_len) {      // no data to send received, yet. dmx not active...
      // let's see if we're connected at all
      if (usb_state) {
        sleepIfIdle();  // if there's been no activity on USB for > 3ms, put CPU to sleep
      }
    }


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
