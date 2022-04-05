#include "dmxusb-dmx-lib-client.h"

// AVR Libc (see http://www.nongnu.org/avr-libc/)
#include <string.h>        // memcpy
#include <avr/io.h>        // include I/O definitions (port names, pin names, etc)
#include <avr/pgmspace.h>  // include program space (for PROGMEM)
#include <avr/interrupt.h> // include interrupt support
#include <avr/wdt.h>       // include watchdog timer support
#include <avr/sleep.h>     // include cpu sleep support
#include <util/delay.h>

// dmx-related globals
u08 dmx_data[NUM_CHANNELS];

#if !DISABLE_DMX_OUTPUT
u16 cur_sending_channel;
u08 client_state;

ISR(USART_UDRE_vect) {
  // Enable interrupts, but first disable the UDRE interrupt to avoid an endless loop.
  // Also, see long comment in dmxusb-dmx-lib-real.cpp.
  cbi(UCSR0B, UDRIE0);
  sei();

  switch(client_state) {
    case client_SyncByte1:
      UDR0 = SYNC_BYTE_1;
      client_state = client_SyncByte2;
      break;
    case client_SyncByte2:
      UDR0 = SYNC_BYTE_2;
      cur_sending_channel = 0;
      client_state = client_Data;
      break;
    case client_Data:
      UDR0 = dmx_data[cur_sending_channel];
      if(++ cur_sending_channel == NUM_CHANNELS)
        client_state = client_SyncByte1;
      break;
  }

  // Re-enable UDRIE0, but first disable interrupts:
  // We only want to handle the next UDRE after returning from this iteration
  // Also, see long comment in dmxusb-dmx-lib-real.cpp.
  cli();
  sbi(UCSR0B, UDRIE0);
}

#endif // !DISABLE_DMX_OUTPUT

void dmx_init() {
#if !DISABLE_DMX_OUTPUT
  client_state = client_SyncByte1;
  
  // init uart
  UBRR0L = F_CPU / (250000 * 16) - 1; UBRR0H =  0; // baud rate 250kbps
  UCSR0A = 0; // clear error flags
  UCSR0C = BV(USBS0) | BV(UCSZ01) | BV(UCSZ00); // 8 data bits, 2 stop bits, no parity (8N2)
  // turn on UART and set UDRIE0 (TX Data Register Empty Interrupt Enable)
  UCSR0B = BV(TXEN0) | BV(UDRIE0);
#endif // !DISABLE_DMX_OUTPUT

  memset(dmx_data, 0, NUM_CHANNELS);
}

void dmx_set_channel(u16 channel, u08 value) {
  dmx_data[channel] = value;
}

void dmx_set_range(u16 channel_start, u16 length, u08 *value_buf) {
  memcpy(&dmx_data[channel_start], value_buf, length);
}

u08 dmx_get_channel(u16 channel) {
  return dmx_data[channel];
}

void dmx_get_range(u16 channel_start, u16 length, u08 *value_buf) {
  memcpy(value_buf, &dmx_data[channel_start], length);
}
