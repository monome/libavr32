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


void test_arp_init(void) {
}


int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_arp_init);

	return UNITY_END();
}

#include "random.c" // FIXME: figure out the proper linking


// mock functions; figure out a better way

u32 time_now(void) {
	return 123909;
}
