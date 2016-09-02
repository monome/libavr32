#include "unity.h"

// this
#include "arp.c"


// chord structure
// -- maintain insert order

// arp structure
// -- build up seq
// -- build down seq
// -- build up/down seq
// -- etc

// arp playback
// -- reset goes to beginning
// -- loop loops
// -- loop loops once per octave, note values increase by octave

chord_t c;

void setup_chord(void) {
	chord_init(&c);
}

void test_chord_init(void) {
	setup_chord();
	TEST_ASSERT_EQUAL_UINT8(0, c.note_count);
}


void test_chord_note_add_maintains_low_to_high_order(void) {
	setup_chord();

	// insert first
	chord_note_add(&c, 100, 0);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.notes[0].num, 100,
																	"0th note number doesn't match after one note add");

	// insert before; shift up
	chord_note_add(&c, 90, 0);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.notes[0].num, 90,
																	"lower note not first");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.notes[1].num, 100,
																	"previous note not pushed up one position");

	// insert after; no shift
	chord_note_add(&c, 110, 0);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.notes[2].num, 110,
																	"highest note not last");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.notes[1].num, 100,
																	"middle note shifted");

	// insert between; some shift
	chord_note_add(&c, 105, 0);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.notes[2].num, 105,
																	"third highest note isn't third note in array");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.notes[3].num, 110,
																	"highest note not last");

}

void test_chord_note_add_dumps_notes_at_capacity(void) {
	bool added;
	u8 i;

	setup_chord();

	for (i = 0; i < CHORD_MAX_NOTES; i++) {
		added = chord_note_add(&c, i, i);
		TEST_ASSERT_TRUE_MESSAGE(added, "note add under capacity should return true");
		TEST_ASSERT_EQUAL_UINT_MESSAGE(i + 1, c.note_count, "note count should be one more than index");
	}

	i = c.note_count;
	added = chord_note_add(&c, 99, 99);
	TEST_ASSERT_FALSE_MESSAGE(added, "adding too many notes should return false");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.note_count, i,
																	"note count should stay the same on overflow");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.note_count, CHORD_MAX_NOTES,
																	"note count should be CHORD_MAX_NOTES on overflow");
}

void test_chord_note_low_and_high(void) {
	setup_chord();

	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_low(&c), -1, "low note when empty should be -1");
	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_high(&c), -1, "high note when empty should be -1");

	chord_note_add(&c, 34, 0);
	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_low(&c), 34, "low note should match only note");
	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_high(&c), 34, "high note should match only note");

	chord_note_add(&c, 40, 0);
	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_low(&c), 34, "low note should match first low note");
	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_high(&c), 40, "high note should match second higher note");
}

void test_chord_note_release(void) {
	setup_chord();

	// empty
	TEST_ASSERT_FALSE_MESSAGE(chord_note_release(&c, 99),
														"release note should return false for empty chord");

	// add, release one
	chord_note_add(&c, 99, 0);
	TEST_ASSERT_EQUAL_UINT8(c.note_count, 1);
	TEST_ASSERT_FALSE_MESSAGE(chord_note_release(&c, 3),
														"release non matching note should return false");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.note_count, 1,
																	"release non matching note shouldn't reduce note count");
	TEST_ASSERT_TRUE_MESSAGE(chord_note_release(&c, 99),
														"release note should return true when note found");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(c.note_count, 0,
																	"releasing matching note should decrement note count");

	// release highest, no shift
	chord_note_add(&c, 2, 0);
	chord_note_add(&c, 1, 0);
	chord_note_add(&c, 3, 0);
	chord_note_release(&c, 3);
	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_low(&c), 1, "lowest should be the lowest note added");
	TEST_ASSERT_EQUAL_INT8_MESSAGE(chord_note_high(&c), 2, "highest should be the second highest added");
	TEST_ASSERT_EQUAL_UINT8(c.note_count, 2);

	chord_note_add(&c, 4, 0);
	chord_note_release(&c, 2); // should leave; 1, 4
	TEST_ASSERT_EQUAL_INT8(chord_note_low(&c), 1);
	TEST_ASSERT_EQUAL_INT8(chord_note_high(&c), 4);
	TEST_ASSERT_EQUAL_UINT8(c.note_count, 2);

	chord_note_release(&c, 1); // should leave; 4
	TEST_ASSERT_EQUAL_INT8(chord_note_low(&c), 4);
	TEST_ASSERT_EQUAL_UINT8(c.note_count, 1);
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_chord_init);
	RUN_TEST(test_chord_note_add_maintains_low_to_high_order);
	RUN_TEST(test_chord_note_add_dumps_notes_at_capacity);
	RUN_TEST(test_chord_note_low_and_high);
	RUN_TEST(test_chord_note_release);

	return UNITY_END();
}

#include "random.c" // FIXME: figure out the proper linking


// mock functions; figure out a better way

u32 time_now(void) {
	return 123909;
}
