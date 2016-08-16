#include "random.h"
#include "timers.h"

#include "arp.h"

//
// prototypes
//
static void arp_seq_build_up(arp_seq_t *s);
static void arp_seq_build_down(arp_seq_t *s);
static void arp_seq_build_converge(arp_seq_t *s);
static void arp_seq_build_diverge(arp_seq_t *s);
static void arp_seq_build_random(arp_seq_t *s);
static void arp_seq_build_played(arp_seq_t *s);


//
// chord implementaion; pick different name? as values aren't harmonically related... yet
//
void chord_init(chord_t *c) {
	for (u8 i = 0; i < CHORD_MAX_NOTES; i++) {
		c->notes[i].num = CHORD_NOTE_MAX;
		c->notes[i].vel = CHORD_VELOCITY_MAX;
	}
	c->note_count = 0;
}

bool chord_note_add(chord_t *c, u8 num, u8 vel) {
	u8 i, j;

	if (c->note_count == CHORD_MAX_NOTES) {
		return false;
	}

	// empty, insert head
	if (c->note_count == 0) {
		c->notes[0].num = num;
		c->notes[0].vel = vel;
		c->note_count++;
		return true;
	}

	// not empty, find insert point
	for (i = 0; i < c->note_count; i++) {
		if (num == c->notes[i].num) {
			// matches existing note, nothing to do
			return false;
		}

		if (num < c->notes[i].num) break;
	}

	// shift up to make space
	for (j = c->note_count; j > i; j--) {
		c->notes[j] = c->notes[j - 1];
	}

	// insert new values
	c->notes[i].num = num;
	c->notes[i].vel = vel;
	c->note_count++;

	return true;
}

bool chord_note_release(chord_t *c, u8 num) {
	u8 i, j;
	bool found = false;

	// find index for num
	for (i = 0; i < c->note_count; i++) {
		if (c->notes[i].num == num) {
			found = true;
			break;
		}
	}

	// shift down (to overwrite)
	if (found) {
		for (j = i; j < c->note_count; j++) {
			c->notes[j] = c->notes[j + 1];
		}
		c->note_count--;
	}

	return found;
}

s8 chord_note_low(chord_t *c) {
	return c->note_count ? c->notes[0].num : -1;
}

s8 chord_note_high(chord_t *c) {
	return c->note_count ? c->notes[c->note_count - 1].num : -1;
}


//
// arp implementation

void arp_seq_init(arp_seq_t* s) {
	chord_init(&(s->chord));

	s->style = eStyleUp;

	for (u8 i = 0; i < ARP_MAX_LENGTH; i++) {
		s->seq[i].empty = 1;
		s->seq[i].note_idx = 0;
		s->seq[i].gate_length = 32; // ~1/4 of a beat?
	}
}

bool arp_seq_note_add(arp_seq_t *s, u8 num, u8 vel) {
	// adds a note to the chord, rebuilds seq?, resets seq
	return false;
}

bool arp_seq_note_release(arp_seq_t *s, u8 num) {
	// removes a note from the chord, rebuilds seq, resets position
	return false;
}

void arp_seq_build(arp_seq_t *s, arp_style style) {

	for (u8 i = 0; i < ARP_MAX_LENGTH; i++) {
		s->seq[i].empty = 1;
	}

	switch (style) {
	case eStyleUp:
		arp_seq_build_up(s);
		break;
	case eStyleDown:
		arp_seq_build_down(s);
		break;
	case eStyleUpDown:
		arp_seq_build_up(s);
		break;
	case eStyleUpAndDown:
		// TODO
		break;
	case eStyleConverge:
		arp_seq_build_converge(s);
		break;
	case eStyleDiverge:
		arp_seq_build_diverge(s);
		break;
	case eStyleRandom:
		arp_seq_build_random(s);
		break;
	case eStylePlayed:
		// TODO: we loose the note order currently
		arp_seq_build_played(s);
		break;
	}
}

static void arp_seq_build_up(arp_seq_t *s) {
	s->style = eStyleUp;
	for (u8 u = 0; u < s->chord.note_count; u++) {
		s->seq[u].note_idx = u;
		s->seq[u].gate_length = 32; // TODO: figure out how this is determined/manipulated
		s->seq[u].empty = 0;
	}		
}

static void arp_seq_build_down(arp_seq_t *s) {
	s->style = eStyleUp;
	u8 d = s->chord.note_count - 1;
	for (u8 i = 0; i < s->chord.note_count; i++) {
		s->seq[d].note_idx = i;
		s->seq[d].gate_length = 32; // TODO: figure out how this is determined/manipulated
		s->seq[d].empty = 0;
		d--;
	}		
}

static void arp_seq_build_converge(arp_seq_t *s) {
}

static void arp_seq_build_diverge(arp_seq_t *s) {
}

static void arp_seq_build_random(arp_seq_t *s) {
	random_state_t r;
	s16 ri;
	u8 count = s->chord.note_count;

	s->style = eStyleRandom;

	random_init(&r, time_now(), 0, count - 1);

	// go through each note, pick a random index within the sequence,
	// place note at next free index...
	for (u8 i = 0; i < count; i++) {
		ri = random_next(&r);
		while (!s->seq[ri].empty) {
			ri++;
			if (ri == count) ri = 0;
		}
		s->seq[ri].note_idx = i;
		s->seq[ri].gate_length = 32; // TODO: figure out how this is determined/manipulated
		s->seq[ri].empty = 0;
	}
}

static void arp_seq_build_played(arp_seq_t *s) {
}


void arp_player_init(arp_player_t *p) {
	p->ch = 0;
	p->position = p->initial = 0;
	p->octaves = 1;

	p->velocity = eVelocityOriginal;
	p->gate = eGateFixed;

	p->latch = false;
}

void arp_player_advance(arp_player_t *p, arp_seq_t *s, midi_behavior_t *b) {
	// move forward one tick (where tick == 1/4? or 1/16)
	// ...call midi_note_on, midi_note_off as needed
}

void arp_player_reset(arp_player_t *p) {
	p->position = p->initial;
}
