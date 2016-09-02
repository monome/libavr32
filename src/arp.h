#ifndef __ARP__
#define __ARP__

#include "notes.h"
#include "midi_common.h"

#ifndef TEST
#include "compiler.h"   // for bool; shouldn't this be "types.h"
#else
#include "test_bool.h"
#endif

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
	eStylePlayed
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

	arp_velocity velocity;
	arp_gate gate;

	u8 fixed_velocity;    // fixed velocity value [0-127]
	u8 fixed_gate;        // fixed gate length [0-??]

	s8 active_note;       // if > 0 the note which is actively playing
	u8 active_gate;       // gate length (in ticks) for active note
	
	bool latch;           //
} arp_player_t;


//-----------------------------
//----- functions

void chord_init(chord_t *c);
bool chord_note_add(chord_t *c, u8 num, u8 vel);
bool chord_note_release(chord_t *c, u8 num);
s8   chord_note_low(chord_t *c);
s8   chord_note_high(chord_t *c);

void arp_seq_init(arp_seq_t *s);
bool arp_seq_set_state(arp_seq_t *s, arp_seq_state state);
arp_seq_state arp_seq_get_state(arp_seq_t *s);
void arp_seq_build(arp_seq_t *a, arp_style style, chord_t *c);

void arp_player_init(arp_player_t* p);
void arp_player_pulse(arp_player_t *p, arp_seq_t *s, midi_behavior_t *b);
void arp_player_reset(arp_player_t *a);

#endif // __ARP__
