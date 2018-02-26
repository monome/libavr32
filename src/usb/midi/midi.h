#ifndef _USB_MIDI_H_
#define _USB_MIDI_H_

#include "types.h"
#include "uhc.h"




// read and spawn events (non-blocking)
extern void midi_read(void);

// write to MIDI device
extern bool midi_write(const u8* data, u32 bytes);
extern void midi_write_packet(u8 cable_number, u8 *pack);

// MIDI device was plugged or unplugged
extern void midi_change(uhc_device_t* dev, u8 plug);


#endif
