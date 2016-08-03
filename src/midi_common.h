#ifndef _MIDI_COMMON_H_
#define _MIDI_COMMON_H_

#include "types.h"

//-----------------------------
//----- types

// midi behavior function types
typedef void (*midi_note_on_t)(u8 ch, u8 num, u8 vel);
typedef void (*midi_note_off_t)(u8 ch, u8 num, u8 vel);
typedef void (*midi_channel_pressure_t)(u8 ch, u8 val);
typedef void (*midi_pitch_bend_t)(u8 ch, u16 bend);
typedef void (*midi_control_change_t)(u8 ch, u8 num, u8 val);

typedef struct {
	midi_note_on_t          note_on;
	midi_note_off_t         note_off;
	midi_channel_pressure_t channel_pressure;
	midi_pitch_bend_t       pitch_bend;
	midi_control_change_t   control_change;

	// TODO: system real-time, panic, etc.
} midi_behavior_t;

//-----------------------------
//----- types

void midi_packet_parse(midi_behavior_t *b, u32 data);

#endif
