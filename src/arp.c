// libavr32
#include "print_funcs.h"

#include "random.h"
#include "timers.h"
#include "util.h"

#include "euclidean/euclidean.h"

#include "interrupts.h"

// this
#include "arp.h"

//-----------------------------
//----- functions

static void arp_seq_build_up(arp_seq_t *s, chord_t *c);
static void arp_seq_build_down(arp_seq_t *s, chord_t *c);
static void arp_seq_build_up_down(arp_seq_t *s, chord_t *c, arp_style style);
static void arp_seq_build_converge(arp_seq_t *s, chord_t *c);
static void arp_seq_build_diverge(arp_seq_t *s, chord_t *c);
static void arp_seq_build_random(arp_seq_t *s, chord_t *c);
static void arp_seq_build_played(arp_seq_t *s, note_pool_t *n);


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

bool chord_contains(chord_t *c, u8 num) {
	for (u8 i = 0; i < c->note_count; i++) {
		if (c->notes[i].num == num) {
			return true;
		}
	}
	return false;
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
	u8 default_gate = ARP_PPQ;
	
	for (u8 i = 0; i < ARP_MAX_LENGTH; i++) {
		s->notes[i].note.num = 0;
		s->notes[i].note.vel = 0;
		s->notes[i].empty = 1;
		s->notes[i].gate_length = default_gate;
	}
	s->length = 0;
	s->state = eSeqFree;
}

bool arp_seq_set_state(arp_seq_t *s, arp_seq_state state) {
	// disable timer interrupts
	u8 irq_flags = irqs_pause();

	bool result = false;

	s->state = state;
	result = true;

	// enable timer interrupts
	irqs_resume(irq_flags);

	return result;
}

arp_seq_state arp_seq_get_state(arp_seq_t *s) {
	return s->state;
}
	
void arp_seq_build(arp_seq_t *s, arp_style style, chord_t *c, note_pool_t *n) {
	for (u8 i = 0; i < ARP_MAX_LENGTH; i++) {
		s->notes[i].empty = 1;
	}

	switch (style) {
	case eStyleUp:
		arp_seq_build_up(s, c);
		break;
	case eStyleDown:
		arp_seq_build_down(s, c);
		break;
	case eStyleUpDown:
	case eStyleUpAndDown:
		arp_seq_build_up_down(s, c, style);
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
		if (n) {
			arp_seq_build_played(s, n);
		}
		break;
	default:
		break;
	}

	/*
	print_dbg("\r\n > arp_seq_build: ");
	for (u8 q = 0; q < s->length; q++) {
		print_dbg_ulong(s->notes[q].note.num);
		print_dbg(" ");
	}
	*/
}

static void arp_seq_build_up(arp_seq_t *s, chord_t *c) {
	for (u8 u = 0; u < c->note_count; u++) {
		s->notes[u].note = c->notes[u];
		s->notes[u].gate_length = 1;
		s->notes[u].empty = 0;
	}

	s->style = eStyleUp;
	s->length = c->note_count;
}

static void arp_seq_build_down(arp_seq_t *s, chord_t *c) {
	u8 d = c->note_count - 1;
	for (u8 i = 0; i < c->note_count; i++) {
		s->notes[d].note = c->notes[i];
		s->notes[d].gate_length = 1;
		s->notes[d].empty = 0;
		d--;
	}

	s->style = eStyleDown;
	s->length = c->note_count;
}

static void arp_seq_build_up_down(arp_seq_t *s, chord_t *c, arp_style style) {
	s8 i, u, d;

	d = (style == eStyleUpDown) ? 1 : 0;

	u = 0;
	for (i = 0; i < c->note_count; i++) {
		s->notes[u].note = c->notes[i];
		s->notes[u].gate_length = 1;
		s->notes[u].empty = 0;
		u++;
	}
	if (c->note_count > 1) {
		for (i = c->note_count - 1 - d; i >= 0 + d; --i) {
			s->notes[u].note = c->notes[i];
			s->notes[u].gate_length = 1;
			s->notes[u].empty = 0;
			u++;
		}
	}

	s->style = style;
	s->length = u;
}

static void arp_seq_build_converge(arp_seq_t *s, chord_t *c) {
	bool take_high;
	u8 low_idx, high_idx, i, j;

	if (c->note_count > 0) {
		i = 0;
		low_idx = 0;
		high_idx = c->note_count - 1;
		take_high = false;
		do {
			j = take_high ? high_idx-- : low_idx++;
			take_high = !take_high;
			s->notes[i].note = c->notes[j];
			s->notes[i].gate_length = 1;
			s->notes[i].empty = 0;
			i++;
		} while (low_idx <= high_idx);
	}

	s->style = eStyleConverge;
	s->length = c->note_count;
}

static void arp_seq_build_diverge(arp_seq_t *s, chord_t *c) {
	bool take_high;
	u8 low_idx, high_idx, i, j;

	if (c->note_count == 1) {
		s->notes[0].note = c->notes[0];
		s->notes[0].gate_length = 1;
		s->notes[0].empty = 0;
	}
	else if (c->note_count > 1) {
		low_idx = c->note_count >> 1;
		high_idx = low_idx + 1;

		if (c->note_count % 2 == 0) {
			low_idx--;
			high_idx--;
		}

		take_high = false;
		for (i = 0; i < c->note_count; i++) {
			j = take_high ? high_idx++ : low_idx--;
			take_high = !take_high;
			s->notes[i].note = c->notes[j];
			s->notes[i].gate_length = 1;
			s->notes[i].empty = 0;
		}
	}

	s->style = eStyleDiverge;
	s->length = c->note_count;
}

static void arp_seq_build_random(arp_seq_t *s, chord_t *c) {
	random_state_t r;
	s16 ri;
	u8 count, upper, i;

	count = c->note_count;
	upper = (count > 0) ? count - 1 : 0;

	random_init(&r, time_now(), 0, upper);

	// ensure empty starts in a known state
	for (i = 0; i < count; i++) {
		s->notes[i].empty = 1;
	}

	// go through each note, pick a random index within the sequence,
	// place note at next free index...
	for (i = 0; i < count; i++) {
		ri = random_next(&r);
		while (!s->notes[ri].empty) {
			ri++;
			if (ri == count) ri = 0;
		}
		s->notes[ri].note = c->notes[i];
		s->notes[ri].gate_length = 1;
		s->notes[ri].empty = 0;
	}

	s->style = eStyleRandom;
	s->length = count;
}

static void arp_seq_build_played(arp_seq_t *s, note_pool_t *p) {
	note_pool_iter_t i;
	const held_note_t *n;
	u8 pos, c;

	// NB: note pools are a linked list in most recent first order (an
	// implementation detail) - the order is reverse of what we need
	// here so the sequence is built in reverse...

	notes_iter_init(&i, p);

	c = notes_count(p);
	pos = c - 1;
	n = notes_iter_next(&i);
	while (n) {
		s->notes[pos].note.num = n->num;
		s->notes[pos].note.vel = n->vel;
		s->notes[pos].gate_length = 1;
		s->notes[pos].empty = 0;
		pos--;
		n = notes_iter_next(&i);
	}

	s->style = eStylePlayed;
	s->length = c;
}

void arp_player_init(arp_player_t *p, u8 ch, u8 division) {
	p->ch = ch;

	p->velocity = eVelocityPlayed;
	p->gate = eGateFixed;

	p->fixed_velocity = 127;
	p->fixed_gate = 0;   // [0-division] == [trigger-tie]
	p->fixed_width = 0;  // [0-128] == [trigger-tie]

	p->active_note = -1;
	p->active_gate = 0;

	p->steps = 0;
	p->step_count = 0;
	p->offset = 12;

	arp_player_set_division(p, division, NULL);
	arp_player_set_fill(p, 1);
	arp_player_set_rotation(p, 0);
	arp_player_set_gate_width(p, 0);
	arp_player_reset(p, NULL);
}

void arp_player_set_steps(arp_player_t *p, u8 steps) {
	p->steps = steps;
	if (steps < p->step_count) {
		p->step_count = 0;
	}
}

inline u8 arp_player_get_steps(arp_player_t *p) {
	return p->steps;
}

inline void arp_player_set_offset(arp_player_t *p, s8 offset) {
	p->offset = offset;
	// MAINT: should step_count be reset here?
}

inline s8 arp_player_get_offset(arp_player_t *p) {
	return p->offset;
}

u8 arp_player_set_gate_width(arp_player_t *p, u8 width) {
	u16 gate = (width * p->division) >> 7;

	p->fixed_width = width;
	p->fixed_gate = gate;

	return p->fixed_gate;
}

inline u8 arp_player_get_gate_width(arp_player_t *p) {
	return p->fixed_width;
}

inline void arp_player_set_fill(arp_player_t *p, u8 fill) {
	p->fill = fill;
}

inline u8 arp_player_get_fill(arp_player_t *p) {
	return uclip(p->fill, 1, p->division);
}

void arp_player_set_division(arp_player_t *p, u8 division, midi_behavior_t *b) {
	if (division > 0 && division != p->division) {
		if (b && division < p->division && p->active_note >= 0) {
			b->note_off(p->ch, p->active_note, 0);
			p->active_note = -1;
		}

		p->division = division;
		p->div_count = 0;

		// re-set fill and gate to clip/recompute internal values which are affected
		// by division
		arp_player_set_fill(p, p->fill);
		arp_player_set_gate_width(p, p->fixed_width);
	}
}

inline u8 arp_player_get_division(arp_player_t *p) {
	return p->division;
}

inline void arp_player_set_rotation(arp_player_t *p, s8 r) {
	p->rotation = r;
}

inline s8 arp_player_get_rotation(arp_player_t *p) {
	return p->rotation;
}

bool arp_player_at_end(arp_player_t *p, arp_seq_t *s) {
	return p->index >= s->length - 1;
}

void arp_player_pulse(arp_player_t *p, arp_seq_t *s, midi_behavior_t *b, u8 phase) {
	u8 i, g, v, f;

	if (phase) {
		f = arp_player_get_fill(p);

		if (euclidean(f, p->division, p->div_count - p->rotation)) {
			// release any active note
			if (p->active_note >= 0) {
				// TODO: how to handle tied note?
				b->note_off(p->ch, p->active_note, 0);
				p->active_note = -1;
			}

			if (s->length > 0) {
				// ensure if seq got shorter we don't go off the end
				if (p->index >= s->length) {
					p->index = 0;
					p->step_count = 0;
				}

				i = p->index;

				// determine velocity
				switch (p->velocity) {
				case eVelocityPlayed:
					v = s->notes[i].note.vel;
					break;
				case eVelocityFixed:
				default:
					v = p->fixed_velocity;
					break;
				}

				// determine gate length
				switch (p->gate) {
				case eGateVariable:
					g = s->notes[i].gate_length;
				case eGateFixed:
				default:
					g = p->fixed_gate;
					break;
				}

				p->active_note = uclip(s->notes[i].note.num + (p->step_count * p->offset),
															 0, MIDI_NOTE_MAX);
				p->active_gate = g;
				b->note_on(p->ch, p->active_note, v);

				// advance seq
				p->index++;

				if (p->index >= s->length) {
					p->index = 0;
					p->step_count++;
					if (p->step_count > p->steps) {
						p->step_count = 0;
					}
				}
			}
		}

		// always advance div_count
		p->div_count++;
		if (p->div_count >= p->division) {
			p->div_count = 0;
		}
	}
	else {
		// clock low, look for notes to turn off
		if (p->active_note >= 0 && p->div_count >= p->active_gate) {
			b->note_off(p->ch, p->active_note, 0);
			p->active_note = -1;
		}
	}
}

void arp_player_reset(arp_player_t *p, midi_behavior_t *b) {
	p->index = 0;
	p->div_count = 0;
	p->step_count = 0;

	if (b && p->active_note >= 0) {
		b->note_off(p->ch, p->active_note, 0);
		p->active_note = -1;
	}
}
