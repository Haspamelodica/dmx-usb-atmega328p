#include "dmx-lib.h"

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
static u16 out_idx;     // index of next frame to send
#endif
static u16 packet_len = 0;  // we only send frames up to the highest channel set
static volatile u08 dmx_state = dmx_Off;

void dmx_init() {
  dmx_state = dmx_Off;

  memset(dmx_data, 0, NUM_CHANNELS);

#if !DISABLE_DMX_OUTPUT
  // init timer0 for DMX timing
  TCCR0B = 2; // prescaler 8 => 1 clock is 2/3 us for 12Mhz / .05 us for 16MHz (8 / F_CPU seconds)

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
#endif // !DISABLE_DMX_OUTPUT
}

#if !DISABLE_DMX_OUTPUT

#if HANDLE_TXC0_VIA_INTERRUPT
#if DEBUG_UNEXPECTED_TXC0
uint8_t txc0_expected;
#endif
uint8_t txc0_from_interrupt;
// Transmit Complete interrupt vector

// TODO see UDRE ISR
ISR(USART_TX_vect, ISR_NOBLOCK) {
#if DEBUG_UNEXPECTED_TXC0
  if (!txc0_expected)
    LED_GREEN_PORT ^= BV(LED_GREEN_BIT);
#endif
  txc0_from_interrupt = 1;
}
#endif // HANDLE_TXC0_VIA_INTERRUPT

// Returns 1 if state is now end-of-packet
inline uint8_t handle_udre_inPacket() {
  // send next byte of dmx packet
  if (out_idx < packet_len) {
    UDR0 = dmx_data[out_idx++];
    return 0;
  }
  else {
#if HANDLE_UDRE0_VIA_INTERRUPT
	cbi(UCSR0B, UDRIE0); // disable UDRE (TX Data Register Empty) interrupt
#endif
#if (HANDLE_TXC0_VIA_INTERRUPT && DEBUG_UNEXPECTED_TXC0)
    txc0_expected = 1;
#endif
    dmx_state = dmx_EndOfPacket;
    return 1;
  }
}

// From usbdrv.h:
// "The application must ensure that the USB interrupt is not disabled for more
// than 25 cycles (this is for 12 MHz, faster clocks allow longer latency)."
// However, we cannot execute sei() first,
// because the UDRE interrupt would immediately be triggered again.
// So, we temporarily disable UDRIE0 and only then enable interrupts.
//
// Also, it seems that 25 cycles is enough time either way:
// According to the Atmega328p datasheet,
// calling an interrupt takes 7 cycles (4 interrupt + 3 jump).
// (Multi-cycle instructions won't be interrupted. See below.)
// CBI takes 1 or 2 cycles (total 9).
// SEI takes 1 cycle (total 10). The following instruction will be executed
// before the next interrupt. In our case, the instruction after the SEI
// is not a RET or RETI (probably a jump), meaning this takes at most 3 cycles (total 13).
// In summary, once the UDRE interrupt is triggered,
// no other interrupts will be able to run for 13 cycles; less than 25.
//
// Additionally, interrupts may not be able to run for some cycles
// before an interrupt is triggered.
// For this, there are three scenarios:
// 1.
// The end of the UDRE ISR starts with CLI, which takes 1 cycle (total 1).
// Next, SBI takes 1 or 2 cycles (total 2).
// Next, the RETI takes 4 cycles (total 6).
// The next instruction after RETI will always be executed.
// In the worst case, it is a RET (total 10).
// Together with the UDRE ISR start code, this is 23 cycles; less than 25.
// (The times of the start and end of the UDRE ISR probably don't even have to be added:
// The USB interrupt is INT0, which takes precedence
// over all other interrupts (except RESET).)
// 2.
// Multi-cycle instructions won't be interrupted.
// According to the ISR, the longest instructions are RET / RETI with 4 cycles.
// This means that if the interrupt occurs while in a multi-byte instruction,
// additional 3 cycles have to be added. Together with UDRE ISR start,
// this takes 16 cycles; less than 25.
// 3.
// Sleep wakeup takes additional 4 cycles, resulting in 17 cycles total; less than 25.
#if HANDLE_UDRE0_VIA_INTERRUPT
ISR(USART_UDRE_vect) {
#if !ATOMIC_UDRE
  // Enable interrupts, but first disable the UDRE interrupt to avoid an endless loop.
  // Also, see long comment above.
  cbi(UCSR0B, UDRIE0);
  sei();
#endif

  while(UCSR0A & BV(UDRE0))
    if(dmx_state == dmx_InPacket)
	  if(handle_udre_inPacket())
		// return, not break, to skip re-enabling UDRIE0 if we are now in EndOfPacket
		return;

#if !ATOMIC_UDRE
  // Re-enable UDRIE0, but first disable interrupts:
  // We only want to handle the next UDRE after returning from this iteration
  // Also, see long comment above.
  cli();
  sbi(UCSR0B, UDRIE0);
#endif
}
#endif // HANDLE_UDRE0_VIA_INTERRUPT

#endif // !DISABLE_DMX_OUTPUT

void dmx_poll() {
#if !DISABLE_DMX_OUTPUT
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
#endif // HANDLE_TXC0_VIA_INTERRUPT
#if HANDLE_UDRE0_VIA_INTERRUPT
		sbi(UCSR0B, UDRIE0); // enable UDRE (TX Data Register Empty) interrupt
#endif // HANDLE_UDRE0_VIA_INTERRUPT
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
#endif // HANDLE_TXC0_VIA_INTERRUPT
          // send a BREAK:
          cbi(UCSR0B, TXEN0); // disable UART transmitter
          cbi(PORTD, 1);    // pull TX pin low

          sbi(GTCCR, PSRSYNC);  // reset timer prescaler
          // TCNT0 = 123;    // 132 clks = 88us // only correct for 12Mhz
          TCNT0 = F_CPU / 8 * 92 / 1000000; // prescaler is 8 and we want 92us (instead of 88us in the original)
          sbi(TIFR0, TOV0); // clear timer overflow flag
          dmx_state = dmx_InBreak;
        }
        break;
      }
    case dmx_InBreak: {
        if (TIFR0 & BV(TOV0)) {
          // end of BREAK: send MARK AFTER BREAK
          sbi(PORTD, 1);    // pull TX pin high
          sbi(GTCCR, PSRSYNC);  // reset timer prescaler
          // TCNT0 = 243;    // 12 clks = 8us // only correct for 12Mhz
          TCNT0 = F_CPU / 8 * 12 / 1000000; // prescaler is 8 and we want 12us (instead of 8us in the original)
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
#endif // !DISABLE_DMX_OUTPUT
}

void dmx_set_channel(u16 channel, u08 value) {
  dmx_data[channel] = value;
  // update dmx state
  if (channel >= packet_len) packet_len = channel;
  if (dmx_state == dmx_Off) dmx_state = dmx_NewPacket;
}

void dmx_set_range(u16 channel_start, u16 length, u08 *value_buf) {
  memcpy(&dmx_data[channel_start], value_buf, length);
  // update state
  if (channel_start + length > packet_len) packet_len = channel_start + length;
  if (dmx_state == dmx_Off) dmx_state = dmx_NewPacket;
}

u08 dmx_get_channel(u16 channel) {
  return dmx_data[channel];
}

void dmx_get_range(u16 channel_start, u16 length, u08 *value_buf) {
  memcpy(value_buf, &dmx_data[channel_start], length);
}
