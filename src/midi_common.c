#include "midi_common.h"

#ifndef TEST
#include "print_funcs.h"
#endif

void midi_packet_parse(midi_behavior_t *b, u32 data) {
	static u8 com;
	static u8 ch, num, val;
	static u16 bend;

	// print_dbg("\r\nmidi packet: 0x");
	// print_dbg_hex(data);

	// check status byte  
  com = (data & 0xf0000000) >> 28;
  ch  = (data & 0x0f000000) >> 24;

	// print_dbg("\r\nmidi packet ch: ");
	// print_dbg_ulong(ch);

	switch (com) {
  case 0x9:
		// note on
  	num = (data & 0xff0000) >> 16;
  	val = (data &   0xff00) >> 8;
		if (val == 0) {
			// note on with zero velocity is note off (per midi spec)
			if (b->note_off) b->note_off(ch, num, val);
		}
		else {
			if (b->note_on) b->note_on(ch, num, val);
		}
		break;
	case 0x8:
		// note off (with velocity)
    num = (data & 0xff0000) >> 16;
    val = (data &   0xff00) >> 8;
		if (b->note_off) b->note_off(ch, num, val);
		break;
	case 0xd:
		// channel pressure
		val = (data & 0x7f0000) >> 16;
		if (b->channel_pressure) b->channel_pressure(ch, val);
		break;
	case 0xe:
		// pitch bend
		bend = ((data & 0x00ff0000) >> 16) | ((data & 0xff00) >> 1);
		if (b->pitch_bend) b->pitch_bend(ch, bend);
		break;
	case 0xb:
		// control change
		num = (data & 0xff0000) >> 16;
    val = (data &   0xff00) >> 8;
		if (b->control_change) b->control_change(ch, num, val);
		break;
	default:
		// TODO: poly pressure, program change, chanel mode *, rtc, etc
		break;
  }
}

void midi_flags_init(midi_voice_flags_t *f) {
	f->sustain = 0;
	f->legato = 0;
	f->portamento = 0;
}
