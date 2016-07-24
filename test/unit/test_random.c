#include "unity.h"

// this
#include "random.c"

#define BAD_SEED -12345
#define GOOD_SEED 12345

#define VALUE_COUNT 2048

#define MAX_ITERATIONS 100000

#define TRUE 1
#define FALSE 0


void test_random_init_and_seed(void) {
	u16 i;
	random_state_t r;

	random_init(&r, GOOD_SEED, -1000, 1000);

	TEST_ASSERT_EQUAL_INT(-1000, r.min);
	TEST_ASSERT_EQUAL_INT(1000, r.max);
	TEST_ASSERT_EQUAL_INT(GOOD_SEED, r.x);

	s16 values[VALUE_COUNT];

	for (i = 0; i < VALUE_COUNT; i++) {
		values[i] = random_next(&r);
	}

	random_seed(&r, GOOD_SEED);

	TEST_ASSERT_EQUAL_INT(GOOD_SEED, r.x);

	// re-seeding should produce the same sequence
	for (i = 0; i < VALUE_COUNT; i++) {
		TEST_ASSERT_EQUAL_INT(values[i], random_next(&r));
	}

	// different seeds should produce sequences
	int differences = 0;
	random_init(&r, BAD_SEED, -1000, 1000);
	for (i = 0; i < VALUE_COUNT; i++) {
		if (values[i] != random_next(&r)) {
			differences++;
		}
	}
	TEST_ASSERT_TRUE(differences > VALUE_COUNT / 2);
}

void test_random_range(void) {
	u32 i;
	s32 v;
	int got_min, got_max;
	random_state_t r;

	// [0,1]
	random_init(&r, GOOD_SEED, 0, 1);

	got_min = got_max = FALSE;
	i = 0;
	do {
		v = random_next(&r);
		if (v == r.min) {
			got_min = TRUE;
		}
		if (v == r.max) {
			got_max = TRUE;
		}
		i++;
	}
	while (i < MAX_ITERATIONS && !(got_min && got_max));

	TEST_ASSERT_TRUE_MESSAGE(got_min, "didn't hit 0");
	TEST_ASSERT_TRUE_MESSAGE(got_min, "didn't hit 1");


	// [-1,2]
	random_init(&r, GOOD_SEED, -1, 2);

	got_min = got_max = FALSE;
	i = 0;
	do {
		v = random_next(&r);
		if (v == r.min) {
			got_min = TRUE;
		}
		if (v == r.max) {
			got_max = TRUE;
		}
		i++;
	}
	while (i < MAX_ITERATIONS && !(got_min && got_max));

	TEST_ASSERT_TRUE_MESSAGE(got_min, "didn't hit -1");
	TEST_ASSERT_TRUE_MESSAGE(got_min, "didn't hit 2");


	// [-100,-80]
	random_init(&r, GOOD_SEED, -100, -80);

	got_min = got_max = FALSE;
	i = 0;
	do {
		v = random_next(&r);
		if (v == r.min) {
			got_min = TRUE;
		}
		if (v == r.max) {
			got_max = TRUE;
		}
		i++;
	}
	while (i < MAX_ITERATIONS && !(got_min && got_max));

	TEST_ASSERT_TRUE_MESSAGE(got_min, "didn't hit -100");
	TEST_ASSERT_TRUE_MESSAGE(got_min, "didn't hit -80");
}


int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_random_init_and_seed);
	RUN_TEST(test_random_range);

	return UNITY_END();
}
