#ifndef _MIDI_COMMON_H_
#define _MIDI_COMMON_H_

#include "types.h"

#define MAX_VOICE_COUNT 4

#define MIDI_BEND_ZERO 0x2000  // 1 << 13



//-----------------------------
//----- types

// midi behavior function types
typedef void (*midi_note_on_t)(u8 ch, u8 num, u8 vel);
typedef void (*midi_note_off_t)(u8 ch, u8 num, u8 vel);
typedef void (*midi_channel_pressure_t)(u8 ch, u8 val);
typedef void (*midi_pitch_bend_t)(u8 ch, u16 bend);
typedef void (*midi_control_change_t)(u8 ch, u8 num, u8 val);
typedef void (*midi_real_time_t)(void);

typedef struct {
	midi_note_on_t          note_on;
	midi_note_off_t         note_off;
	midi_channel_pressure_t channel_pressure;
	midi_pitch_bend_t       pitch_bend;
	midi_control_change_t   control_change;

	midi_real_time_t        clock_tick;
	midi_real_time_t        seq_start;
	midi_real_time_t        seq_stop;
	midi_real_time_t        seq_continue;
} midi_behavior_t;

typedef struct {
	u8 sustain    : 1;
	u8 legato     : 1;
	u8 portamento : 1;
} voice_flags_t;

typedef enum {
	kVoiceAllocRotate,
	kVoiceAllocLRU
} voice_alloc_mode;

typedef struct {
	u8 num    : 7;
	u8 active : 1;
} voice_t;

typedef struct {
	u8               head;
	u8               tail;
	u8               count;
	voice_t          voice[MAX_VOICE_COUNT];
	voice_alloc_mode mode;
} voice_state_t;

//-----------------------------
//----- types

void midi_packet_parse(midi_behavior_t *b, u32 data);

void voice_flags_init(voice_flags_t *f);

void voice_slot_init(voice_state_t *v, voice_alloc_mode mode, u8 count);
u8   voice_slot_next(voice_state_t *v);
u8   voice_slot_num(voice_state_t *v, u8 slot);
u8   voice_slot_active(voice_state_t *v, u8 slot);
void voice_slot_activate(voice_state_t *v, u8 slot, u8 num);
s8   voice_slot_find(voice_state_t *v, u8 num);
void voice_slot_release(voice_state_t *v, u8 slot);


#endif
