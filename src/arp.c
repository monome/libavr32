#include "random.h"
#include "timers.h"

#include "arp.h"

//-----------------------------
//----- functions

static void arp_seq_build_up(arp_seq_t *s, chord_t *c);
static void arp_seq_build_down(arp_seq_t *s, chord_t *c);
static void arp_seq_build_converge(arp_seq_t *s, chord_t *c);
static void arp_seq_build_diverge(arp_seq_t *s, chord_t *c);
static void arp_seq_build_random(arp_seq_t *s, chord_t *c);
static void arp_seq_build_played(arp_seq_t *s, chord_t *c);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///// chord implementation

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///// arp seq: implementation

void arp_seq_init(arp_seq_t* s) {
	for (u8 i = 0; i < ARP_MAX_LENGTH; i++) {
		s->seq[i].note.num = 0;
		s->seq[i].note.vel = 0;
		s->seq[i].empty = 1;
		s->seq[i].gate_length = 32; // ~1/4 of a beat?
	}
	s->length = 0;
}

void arp_seq_build(arp_seq_t *s, arp_style style, chord_t *c) {
	for (u8 i = 0; i < ARP_MAX_LENGTH; i++) {
		s->seq[i].empty = 1;
	}

	switch (style) {
	case eStyleUp:
		arp_seq_build_up(s, c);
		break;
	case eStyleDown:
		arp_seq_build_down(s, c);
		break;
	case eStyleUpDown:
		arp_seq_build_up(s, c);
		break;
	case eStyleUpAndDown:
		// TODO
		break;
	case eStyleConverge:
		arp_seq_build_converge(s, c);
		break;
	case eStyleDiverge:
		arp_seq_build_diverge(s, c);
		break;
	case eStyleRandom:
		arp_seq_build_random(s, c);
		break;
	case eStylePlayed:
		// TODO: we loose the note order currently
		arp_seq_build_played(s, c);
		break;
	}
}

static void arp_seq_build_up(arp_seq_t *s, chord_t *c) {
	s->style = eStyleUp;
	for (u8 u = 0; u < c->note_count; u++) {
		s->seq[u].note = c->notes[u];
		s->seq[u].gate_length = 8; // TODO: figure out how this is determined/manipulated
		s->seq[u].empty = 0;
	}
	s->length = c->note_count;
}

static void arp_seq_build_down(arp_seq_t *s, chord_t *c) {
	s->style = eStyleDown;
	u8 d = c->note_count - 1;
	for (u8 i = 0; i < c->note_count; i++) {
		s->seq[d].note = c->notes[i];
		s->seq[d].gate_length = 8; // TODO: figure out how this is determined/manipulated
		s->seq[d].empty = 0;
		d--;
	}
	s->length = c->note_count;
}

static void arp_seq_build_converge(arp_seq_t *s, chord_t *c) {
}

static void arp_seq_build_diverge(arp_seq_t *s, chord_t *c) {
}

static void arp_seq_build_random(arp_seq_t *s, chord_t *c) {
	random_state_t r;
	s16 ri;
	u8 count = c->note_count;

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
		s->seq[ri].note = c->notes[i];
		s->seq[ri].gate_length = 8; // TODO: figure out how this is determined/manipulated
		s->seq[ri].empty = 0;
	}
	s->length = count;
}

static void arp_seq_build_played(arp_seq_t *s, chord_t *c) {
	// TODO: need some way to now order within chord
}

void arp_player_init(arp_player_t *p) {
	p->ch = 0;
	p->start = 0;
	p->octaves = 1;

	p->velocity = eVelocityPlayed;
	p->gate = eGateFixed;

	p->fixed_velocity = 127;
	p->fixed_gate = ARP_PPQ - 1;

	p->latch = false;

	arp_player_reset(p);
}

void arp_player_pulse(arp_player_t *p, arp_seq_t *s, midi_behavior_t *b) {
	// move forward one tick (where tick == 1/4? or 1/16) => use 24ppq like midi
	// ...call midi_note_on, midi_note_off as needed
	u8 i, d, g, v;

	if (s->length == 0) {
		return;
	}
	
	p->tick_count++;
	p->div_count++;
	if (p->div_count == ARP_PPQ) {
		// advance note reset
		p->index++;
		p->div_count = 0;
	}
	if (p->index == s->length) {
		// sequence wrap
		p->index = 0;
		p->tick_count = 0;
	}

	i = p->index;
	d = p->div_count;
	
	// which gate length
	switch (p->gate) {
	case eGateVariable:
		g = s->seq[i].gate_length;
	case eGateFixed:
	default:
		g = p->fixed_gate;
		break;
	}		

	// note off if division matches gate length
	if (d == g) {
		b->note_off(p->ch, s->seq[i].note.num, 0);
	}

	// which velocity
	switch (p->velocity) {
	case eVelocityPlayed:
		v = s->seq[i].note.vel;
		break;
	case eVelocityFixed:
	default:
		v = p->fixed_velocity;
		break;
	}

	if (d == 0) {
		b->note_on(p->ch, s->seq[i].note.num, v);
	}
}

void arp_player_reset(arp_player_t *p) {
	p->index = p->start;
	p->div_count = 0;
	p->tick_count = p->index * ARP_PPQ;
}
