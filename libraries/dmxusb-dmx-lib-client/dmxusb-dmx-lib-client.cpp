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

void dmx_init() {
  // TODO
}

void dmx_poll() {
  // TODO
}

void dmx_set_channel(u16 channel, u08 value) {
  dmx_data[channel] = value;
  // TODO update dmx state
}

void dmx_set_range(u16 channel_start, u16 length, u08 *value_buf) {
  memcpy(&dmx_data[channel_start], value_buf, length);
  // TODO update dmx state
}

u08 dmx_get_channel(u16 channel) {
  return dmx_data[channel];
}

void dmx_get_range(u16 channel_start, u16 length, u08 *value_buf) {
  memcpy(value_buf, &dmx_data[channel_start], length);
}
