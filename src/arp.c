#include "random.h"
#include "timers.h"

#include "conf_tc_irq.h"

#include "arp.h"

//-----------------------------
//----- functions

static void arp_seq_build_up(arp_seq_t *s, chord_t *c);
static void arp_seq_build_down(arp_seq_t *s, chord_t *c);
static void arp_seq_build_up_down(arp_seq_t *s, chord_t *c, arp_style style);
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
	bool result = false;
	
	cpu_irq_disable_level(APP_TC_IRQ_PRIORITY);

	s->state = state;
	result = true;
	/*
	if (s->state == eSeqFree) {
		s->state = state; // free, building, playing
		result = true;
	}
	else if (state == eSeqFree) {
		s->state = state; // building -> free or playing -> free
		result = true;
	}
	// invalid transition; build -> play or play -> build
	*/
	cpu_irq_enable_level(APP_TC_IRQ_PRIORITY);
	
	return result;
}

arp_seq_state arp_seq_get_state(arp_seq_t *s) {
	return s->state;
}
	
void arp_seq_build(arp_seq_t *s, arp_style style, chord_t *c) {
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
		// TODO: we loose the note order currently
		arp_seq_build_played(s, c);
		break;
	default:
		break;
	}
}

static void arp_seq_build_up(arp_seq_t *s, chord_t *c) {
	s->style = eStyleUp;
	for (u8 u = 0; u < c->note_count; u++) {
		s->notes[u].note = c->notes[u];
		s->notes[u].gate_length = 1;
		s->notes[u].empty = 0;
	}
	s->length = c->note_count;
}

static void arp_seq_build_down(arp_seq_t *s, chord_t *c) {
	s->style = eStyleDown;
	u8 d = c->note_count - 1;
	for (u8 i = 0; i < c->note_count; i++) {
		s->notes[d].note = c->notes[i];
		s->notes[d].gate_length = 1;
		s->notes[d].empty = 0;
		d--;
	}
	s->length = c->note_count;
}

static void arp_seq_build_up_down(arp_seq_t *s, chord_t *c, arp_style style) {
	s8 i, u, d;

	s->style = style;
	d = (style == eStyleUpDown) ? 1 : 0;

	u = 0;
	for (i = 0; i < c->note_count; i++) {
		s->notes[u].note = c->notes[i];
		s->notes[u].gate_length = 1;
		s->notes[u].empty = 0;
		u++;
	}
	if (c->note_count > 1) {
		for (i = c->note_count - 1 - d; i >= 0; --i) {
			s->notes[u].note = c->notes[i];
			s->notes[u].gate_length = 1;
			s->notes[u].empty = 0;
			u++;
		}
	}
	s->length = u;
}

static void arp_seq_build_converge(arp_seq_t *s, chord_t *c) {
	s->style = eStyleConverge;
	s->length = 0;
}

static void arp_seq_build_diverge(arp_seq_t *s, chord_t *c) {
	s->style = eStyleDiverge;
	s->length = 0;
}

static void arp_seq_build_random(arp_seq_t *s, chord_t *c) {
	random_state_t r;
	s16 ri;
	u8 count, upper, i;

	s->style = eStyleRandom;

	count = c->note_count;
	upper = (count > 0) ? count - 1 : 0;

	random_init(&r, time_now(), 0, upper);

	// ensure empty starts in a known state
	for (i = 0; i < count; i++) {
		s->notes[ri].empty = 1;
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
		s->notes[ri].gate_length = 8; // TODO: figure out how this is determined/manipulated
		s->notes[ri].empty = 0;
	}
	s->length = count;
}

static void arp_seq_build_played(arp_seq_t *s, chord_t *c) {
	// TODO: need some way to now order within chord
	s->style = eStylePlayed;
	s->length = 0;
}

void arp_player_init(arp_player_t *p, u8 ch, u8 division) {
	p->ch = ch;

	p->velocity = eVelocityPlayed;
	p->gate = eGateFixed;

	p->fixed_velocity = 127;

	p->active_note = -1;
	p->active_gate = 0;
	
	p->latch = false;

	arp_player_set_division(p, division);
	arp_player_reset(p, NULL);
}

void arp_player_set_division(arp_player_t *p, u8 division) {
	if (division > 0) {
		p->division = division;
		p->fixed_gate = division - 1;
	}
}

void arp_player_pulse(arp_player_t *p, arp_seq_t *s, midi_behavior_t *b, u8 phase) {
	u8 i, g, v;

	if (phase) {
		if (p->div_count == 0) {
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

				p->active_note = s->notes[i].note.num;
				p->active_gate = g;
				b->note_on(p->ch, p->active_note, v);

				// advance seq
				p->index++;

				if (p->index >= s->length) {
					p->index = 0;
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
		if (p->active_gate <= p->div_count && p->active_note >= 0) {
			b->note_off(p->ch, p->active_note, 0);
			p->active_note = -1;
		}
	}
}

void arp_player_reset(arp_player_t *p, midi_behavior_t *b) {
	p->index = 0;
	p->div_count = 0;

	if (b && p->active_note >= 0) {
		b->note_off(p->ch, p->active_note, 0);
		p->active_note = -1;
	}
}
