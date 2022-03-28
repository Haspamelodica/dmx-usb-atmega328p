#define IS_DMX_SERVER 1

// ==============================================================================
// includes
// ------------------------------------------------------------------------------

// DMX library (includes config)
#include <dmxusb-dmx-lib.h>

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

void init() {
#if DEBUG_ENABLED
  Serial.begin(DEBUG_BAUD);
  dbg_print(F("Init..."));
#endif

  // clear Power On reset flag
  MCUSR &= ~(1 << PORF);

  // configure IO-Ports; most are unused, we set them to outputs to have defined voltages
  DDRB = 0xFF;    // unused
  DDRC = 0xFF;    // unused
  // unused except PD1 (TX) which needs to be an output anyway.
  DDRD = 0xFF;

  wdt_enable(WDTO_1S);  // enable watchdog timer

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
    dmx_poll();
	//TODO receive and handle byte
  }
  return 0;
}
