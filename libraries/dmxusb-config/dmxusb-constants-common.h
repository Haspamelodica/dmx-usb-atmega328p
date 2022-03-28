#include "dmxusb-config-common.h"

#if DMX_CHANNELS > 512
	#error "DMX_CHANNELS is too high; must be at most 512. Change in dmxusb-config-common.h."
#endif

// convenience macros (from Pascal Stangs avrlib)
#ifndef BV
    #define BV(bit)         (1<<(bit))
#endif
#ifndef cbi
    #define cbi(reg,bit)    reg &= ~(BV(bit))
#endif
#ifndef sbi
    #define sbi(reg,bit)    reg |= (BV(bit))
#endif

typedef unsigned char  u08;
typedef   signed char  s08;
typedef unsigned short u16;
typedef   signed short s16;
