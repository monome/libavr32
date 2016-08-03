#include "unity.h"

// this
#include "midi_common.c"

void test_midi_parse_packet(void) {
	TEST_IGNORE();
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_midi_parse_packet);

	return UNITY_END();
}
