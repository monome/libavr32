#include "unity.h"

// this
#include "notes.c"

void test_notes_init(void) {
	// ensure initializer doesn't crash
	notes_init();
}


int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_notes_init);

	return UNITY_END();
}
