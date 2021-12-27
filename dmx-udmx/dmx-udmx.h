#ifndef __udmx_h_included__
#define __udmx_h_included__

#ifndef __ASSEMBLER__

#define ENABLE_SLEEP_IF_IDLE 0
#define DEBUG_USB 0
#define DEBUG_DMX_VALUES 0
#define DEBUG_UNEXPECTED_TXC0 1
#define HANDLE_TXC0_VIA_INTERRUPT 1
#define HANDLE_UDRE0_VIA_INTERRUPT 1

#define NUM_CHANNELS 512		// number of channels in DMX-512

// values for dmx_state
#define dmx_Off 0
#define dmx_NewPacket 1	
#define dmx_InPacket 2
#define dmx_EndOfPacket 3
#define dmx_InBreak 4
#define dmx_InMAB 5

// values for usb_state
#define usb_NotInitialized 0
#define usb_Idle 1
#define usb_ChannelRange 2


// LED constants
#define LED_GREEN_PORT PORTD
#define LED_GREEN_BIT 3
#define LED_YELLOW_PORT PORTD
#define LED_YELLOW_BIT 4


// function prototypes
#ifdef __cplusplus
extern "C"
#endif
void hadAddressAssigned(void);

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
#endif
#endif


