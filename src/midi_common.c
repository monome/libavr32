#include "midi_common.h"

void midi_packet_parse(midi_behavior_t *b, u32 data) {
	static u8 com;
	static u8 ch, num, val;
	static u16 bend;

	// print_dbg("\r\nmidi packet: 0x");
	// print_dbg_hex(data);

	// FIXME: ch seems to always be 0?

	// check status byte  
  com = (data & 0xf0000000) >> 28;
  ch  = (data & 0x0f000000) >> 24;
	switch (com) {
  case 0x9:
		// note on
  	num = (data & 0xff0000) >> 16;
  	val = (data &   0xff00) >> 8;
		if (val == 0)
			// note on with zero velocity is note off (per midi spec)
			b->note_off(ch, num, val);
		else
			b->note_on(ch, num, val);
		break;
	case 0x8:
		// note off (with velocity)
    num = (data & 0xff0000) >> 16;
    val = (data &   0xff00) >> 8;
		b->note_off(ch, num, val);
		break;
	case 0xd:
		// channel pressure
		val = (data & 0x7f0000) >> 16;
		b->channel_pressure(ch, val);
		break;
	case 0xe:
		// pitch bend
		bend = ((data & 0x00ff0000) >> 16) | ((data & 0xff00) >> 1);
		b->pitch_bend(ch, bend);
		break;
	case 0xb:
		// control change
		num = (data & 0xff0000) >> 16;
    val = (data &   0xff00) >> 8;
		b->control_change(ch, num, val);
		break;
	default:
		// TODO: poly pressure, program change, chanel mode *, rtc, etc
		break;
  }
}
