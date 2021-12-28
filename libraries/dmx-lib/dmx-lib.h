#include "dmx-constants.h"

// convenience macros (from Pascal Stangs avrlib)
#ifndef BV
	#define BV(bit)			(1<<(bit))
#endif
#ifndef cbi
	#define cbi(reg,bit)	reg &= ~(BV(bit))
#endif
#ifndef sbi
	#define sbi(reg,bit)	reg |= (BV(bit))
#endif

typedef unsigned char  u08;
typedef   signed char  s08;
typedef unsigned short u16;
typedef   signed short s16;

void dmx_init();
void dmx_poll();
void dmx_set_channel(u16 channel, u08 value);
void dmx_set_range(u16 channel_start, u16 length, u08 *value_buf);

#if DISABLE_DMX_OUTPUT
extern u08 dmx_data[NUM_CHANNELS];
#endif
