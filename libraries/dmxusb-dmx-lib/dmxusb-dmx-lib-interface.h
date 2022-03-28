#include "dmxusb-constants-dmx.h"

void dmx_init();
void dmx_poll();
void dmx_set_channel(u16 channel, u08 value);
void dmx_set_range(u16 channel_start, u16 length, u08 *value_buf);
u08  dmx_get_channel(u16 channel);
void dmx_get_range(u16 channel_start, u16 length, u08 *value_buf);

#if DISABLE_DMX_OUTPUT
extern u08 dmx_data[NUM_CHANNELS];
#endif
