#ifndef __ARP_H__
#define __ARP_H__

#include "notes.h"
#include "midi_common.h"

#include "compiler.h"   // for bool; shouldn't this be "types.h"

#define ARP_MAX_CHORD  12
#define ARP_MAX_OCTAVE 4
#define ARP_MAX_LENGTH (2 * ARP_MAX_CHORD)

#define ARP_PPQ 1

#define CHORD_MAX_NOTES 12
#define CHORD_NOTE_MAX 127
#define CHORD_VELOCITY_MAX 127


//-----------------------------
//----- constants

typedef enum {
	eStyleUp,
	eStyleDown,
	eStyleUpDown,
	eStyleUpAndDown,
	eStyleConverge,     // outside in
	eStyleDiverge,      // inside out
	eStyleRandom,
	eStylePlayed,

	eStyleMax
} arp_style;

typedef enum {
	eVelocityFixed,
	eVelocityPlayed
} arp_velocity;

typedef enum {
	eGateFixed,
	eGateVariable
} arp_gate;

typedef enum {
	eSeqFree,
	eSeqBuilding,
	eSeqWaiting,
	eSeqPlaying
} arp_seq_state;

//-----------------------------
//----- types

typedef struct {
	held_note_t notes[CHORD_MAX_NOTES];
	u8 note_count;
} chord_t;

typedef struct {
	held_note_t note;
	u8 gate_length;    // [0-ARP_PPQ], 0 is tie, 1-ARP_PPQ is fraction of quater note
	u8 empty : 1;
} arp_note_t;

typedef struct {
	arp_style style;
	arp_note_t notes[ARP_MAX_LENGTH];
	u8 length;
	arp_seq_state state;
} arp_seq_t;
	
typedef struct {
	u8 ch;                // channel; passed to midi behavior
	u8 index;             // currrent note index

	u8 division;          // pulses per note
	u16 div_count;        // current note division

	arp_velocity velocity;
	arp_gate gate;

	u8 fixed_velocity;    // fixed velocity value [0-127]
	u8 fixed_gate;        // fixed gate length [0-division]
	u8 fixed_width;       // relative gate width [0-128]

	s8 active_note;       // if > 0 the note which is actively playing
	u8 active_gate;       // gate length (in ticks) for active note
	
	u8 steps;             // number of steps (repeats?) of the arp pattern
	u8 step_count;        // current step number
	s8 offset;            // number of semitones to transpose by per step; [-24,24] or voltage offsets?
} arp_player_t;


//-----------------------------
//----- functions

void chord_init(chord_t *c);
bool chord_contains(chord_t *c, u8 num);
bool chord_note_add(chord_t *c, u8 num, u8 vel);
bool chord_note_release(chord_t *c, u8 num);
s8   chord_note_low(chord_t *c);
s8   chord_note_high(chord_t *c);

void arp_seq_init(arp_seq_t *s);
bool arp_seq_set_state(arp_seq_t *s, arp_seq_state state);
arp_seq_state arp_seq_get_state(arp_seq_t *s);
void arp_seq_build(arp_seq_t *a, arp_style style, chord_t *c);

void arp_player_init(arp_player_t *p, u8 ch, u8 division);
u8 arp_player_set_gate_width(arp_player_t *p, u8 width);
void arp_player_set_division(arp_player_t *p, u8 division, midi_behavior_t *b);
bool arp_player_at_end(arp_player_t *p, arp_seq_t *s);
void arp_player_pulse(arp_player_t *p, arp_seq_t *s, midi_behavior_t *b, u8 phase);
void arp_player_reset(arp_player_t *a, midi_behavior_t *b);

#endif // __ARP_H__
