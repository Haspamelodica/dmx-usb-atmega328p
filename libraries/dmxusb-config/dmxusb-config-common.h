// Maximum number of channels to support.
// Must not be higher than 512, which is the total number of channels in DMX.
#define NUM_CHANNELS 512

// If this is set, the USB interface will run on a different Atmega than the DMX interface.
// In this case two Atmegas with different sketches are needed:
// The first Atmega (the one with the USB interface) runs the dmx-hid / dmx-udmx sketch as usual,
// and the second one (the one with the RS485 adapter) runs the dmx-server sketch.
// The dmx-hid / dmx-udmx sketch needs to be recompiled and reuploaded if this changes.
#define SPLIT_MODE 0

// LED constants
#define LED_GREEN_PORT PORTD
#define LED_GREEN_BIT 3
#define LED_YELLOW_PORT PORTD
#define LED_YELLOW_BIT 4
