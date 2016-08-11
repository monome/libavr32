#ifndef __ARP__
#define __ARP__

#include "notes.h"
#include "midi_common.h"

#ifndef TEST
#include "compiler.h"   // for bool
#else
#include "test_bool.h"
#endif

#define ARP_MAX_CHORD  12
#define ARP_MAX_OCTAVE 4
#define ARP_MAX_LENGTH (2 * ARP_MAX_CHORD)

#define CHORD_MAX_NOTES 12
#define CHORD_NOTE_MAX 127
#define CHORD_VELOCITY_MAX 127

//
// constants
//

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
	eVelocityOriginal
} arp_velocity;

typedef enum {
	eGateFixed,
	eGateVariable
} arp_gate;

//
// types
//

typedef struct {
	held_note_t notes[CHORD_MAX_NOTES];
	u8 note_count;
} chord_t;

typedef struct {
	u8 note_idx;       // [0-CHORD_MAX_NOTES), which note in the code to play
	u8 gate_length;    // [0-127], 0 is tie, 1-127 is fraction of beat (in ticks)?
	u8 empty : 1;
} arp_note_t;

typedef struct {
	chord_t chord;

	arp_style style;
	arp_note_t seq[ARP_MAX_LENGTH];
} arp_seq_t;
	
typedef struct {
	u8 ch;           // channel; passed to midi behavior
	u16 position;    // current position (in ticks)
	u16 initial;     // initial position at start of cycle (in ticks)

	u8 octaves;     // number of octaves

	arp_velocity velocity;
	arp_gate gate;

	bool latch;      //
} arp_player_t;


void chord_init(chord_t *c);
bool chord_note_add(chord_t *c, u8 num, u8 vel);
bool chord_note_release(chord_t *c, u8 num);
s8   chord_note_low(chord_t *c);
s8   chord_note_high(chord_t *c);

void arp_seq_init(arp_seq_t *s);
bool arp_seq_note_add(arp_seq_t *a, u8 num, u8 vel);
bool arp_seq_note_release(arp_seq_t *a, u8 num);
void arp_seq_build(arp_seq_t *a, arp_style style);

// 
void arp_player_init(arp_player_t* p);
void arp_player_advance(arp_player_t *p, arp_seq_t *s, midi_behavior_t *b);
void arp_player_reset(arp_player_t *a);

#endif // __ARP__
