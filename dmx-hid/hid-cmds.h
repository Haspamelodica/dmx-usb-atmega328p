#define BLOCK_SIZE 32

// Commands 0-15 set blocks of output data.
// Set interface mode. For modes see below.
#define cmd_SetMode 16
// Set various timing parameters. Ignored.
#define cmd_SetTimings 17
// Store timings to internal memory, probably? Ignored.
#define cmd_StoreTimings 18

#define mode_in_to_out_bit (1 << 0)
#define mode_pc_to_out_bit (1 << 1)
#define mode_in_to_pc_bit (1 << 2)
#define mode_mask (mode_in_to_out_bit | mode_pc_to_out_bit | mode_in_to_pc_bit)
