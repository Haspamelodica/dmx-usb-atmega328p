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
#include <dmxusb-debug-lib.h>
#endif

u16 cur_receiving_channel;
u08 server_state;

ISR(USART_RX_vect) {
  // Enable interrupts, but first disable the RXC interrupt to avoid an endless loop.
  cbi(UCSR0B, RXCIE0);
  sei();

  switch(server_state) {
    case server_WaitingSyncByte1:
      if(UDR0 == SYNC_BYTE_1)
        server_state = server_WaitingSyncByte2;
      break;
    case server_WaitingSyncByte2:
      if(UDR0 == SYNC_BYTE_2) {
        cur_receiving_channel = 0;
        server_state = server_Data;
      } else
        server_state = server_WaitingSyncByte1;
      break;
    case server_Data:
      dmx_set_channel(cur_receiving_channel, UDR0);
      if(++ cur_receiving_channel == NUM_CHANNELS)
        server_state = server_WaitingSyncByte1;
      break;
  }

  // Re-enable RXCIE0, but first disable interrupts:
  // We only want to handle the next RXC after returning from this iteration
  cli();
  sbi(UCSR0B, RXCIE0);
}

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

  // dmx_init initializes UART baud rate, frame format, and transmitter.
  dmx_init();
  
  // Set up UART receiver: set RXEN0 and RXCIE0 (RX Complete Interrupt Enable) bits
  UCSR0B |= BV(RXEN0) | BV(RXCIE0);
  server_state = server_WaitingSyncByte1;

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
  }
  return 0;
}
