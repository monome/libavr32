#include "unity.h"

// this
#include "midi_common.c"

// non-avr helpers
#include <stdlib.h>

static voice_state_t vs;

void test_midi_parse_packet(void) {
	TEST_IGNORE();
}



///////////////////////////////////////////////////////////////////////////////////
////// voice allocation; init

void test_voice_slot_init(void) {
	// ensure lower bound on count
	voice_slot_init(&vs, kVoiceAllocRotate, 0);
	TEST_ASSERT_EQUAL(kVoiceAllocRotate, vs.mode);
	TEST_ASSERT_EQUAL_UINT8(1, vs.count);

	// ensure upper bount on count
	voice_slot_init(&vs, kVoiceAllocRotate, 255);
	TEST_ASSERT_EQUAL(kVoiceAllocRotate, vs.mode);
	TEST_ASSERT_EQUAL_UINT8(MAX_VOICE_COUNT, vs.count);

	voice_slot_init(&vs, kVoiceAllocLRU, MAX_VOICE_COUNT-1);
	TEST_ASSERT_EQUAL(kVoiceAllocLRU, vs.mode);
	TEST_ASSERT_EQUAL_UINT8(MAX_VOICE_COUNT-1, vs.count);

	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[0].num);
	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[0].active);
	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[MAX_VOICE_COUNT-1].num);
	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[MAX_VOICE_COUNT-1].active);
}

//////////////////////////////////////////////////////////////////////////////////
///// voice allocation; rotate

void setup_voice_slot_rotate(u8 count) {
	voice_slot_init(&vs, kVoiceAllocRotate, count);
}

void test_voice_slot_next_rotate(void) {
	setup_voice_slot_rotate(1);
	TEST_ASSERT_EQUAL_UINT8(0, voice_slot_next(&vs));
	TEST_ASSERT_EQUAL_UINT8(0, voice_slot_next(&vs)); // still should be 0
	TEST_ASSERT_EQUAL_UINT8(0, voice_slot_next(&vs)); // ditto

	setup_voice_slot_rotate(3);
	TEST_ASSERT_EQUAL_UINT8(0, voice_slot_next(&vs));
	TEST_ASSERT_EQUAL_UINT8(1, voice_slot_next(&vs)); // next higher slot
	TEST_ASSERT_EQUAL_UINT8(2, voice_slot_next(&vs)); // next higher slot
	TEST_ASSERT_EQUAL_UINT8(0, voice_slot_next(&vs)); // wraps back to 0
}

void test_voice_slot_activate(void) {
	u8 num = 23;
	setup_voice_slot_rotate(4);

	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[0].active);
	voice_slot_activate(&vs, 0, num);
	TEST_ASSERT_EQUAL_UINT8(1, vs.voice[0].active);
	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[1].active); // should be off still
	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[2].active); // ditto

	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[3].active);
	voice_slot_activate(&vs, 3, num);
	TEST_ASSERT_EQUAL_UINT8(1, vs.voice[3].active); // should be on
	TEST_ASSERT_EQUAL_UINT8(1, vs.voice[0].active); // should still be active
	TEST_ASSERT_EQUAL_UINT8(0, vs.voice[1].active); // should still be off
}

void test_voice_slot_find(void) {
	setup_voice_slot_rotate(4);

	// no match
	TEST_ASSERT_EQUAL_INT8(-1, voice_slot_find(&vs, 123));

	// unique match
	voice_slot_activate(&vs, 0, 64);
	voice_slot_activate(&vs, 1, 65);
	voice_slot_activate(&vs, 3, 66);

	TEST_ASSERT_EQUAL_INT8(0, voice_slot_find(&vs, 64));
	TEST_ASSERT_EQUAL_INT8(1, voice_slot_find(&vs, 65));
	TEST_ASSERT_EQUAL_INT8(3, voice_slot_find(&vs, 66));

	// duplicate num returns first active voice
	voice_slot_activate(&vs, 1, 34);
	voice_slot_activate(&vs, 3, 34);

	TEST_ASSERT_EQUAL_INT8(1, voice_slot_find(&vs, 34));
	voice_slot_release(&vs, 1);
	TEST_ASSERT_EQUAL_INT8(3, voice_slot_find(&vs, 34));
}


void test_voice_slot_release_rotate(void) {
	u8 slot;
	u8 count = 3;

	setup_voice_slot_rotate(count);

	// ensure rotate mode doesn't skip over active slots
	for (int i = 0; i < count * 5; i++) {
		slot = voice_slot_next(&vs);
		// in rotate mode slot number and voice numbers should match
		//TEST_ASSERT_EQUAL_UINT8(i % count, voice_slot_num(&vs, slot));
		voice_slot_activate(&vs, slot, rand() % 127);

		// randomly release a slot
		slot = rand() % count;
		voice_slot_release(&vs, slot);
		TEST_ASSERT_EQUAL_UINT8(0, vs.voice[slot].active);
	}
}


//////////////////////////////////////////////////////////////////////////////////
///// test runner


int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_midi_parse_packet);

	RUN_TEST(test_voice_slot_init);
	RUN_TEST(test_voice_slot_next_rotate);
	RUN_TEST(test_voice_slot_activate);
	RUN_TEST(test_voice_slot_find);
	RUN_TEST(test_voice_slot_release_rotate);

	return UNITY_END();
}
