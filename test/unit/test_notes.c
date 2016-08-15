#include "unity.h"

// this
#include "notes.c"

//
// test internals
//

void test_pool_init(void) {
	// ensure initializer doesn't crash
	u16 canary_before = 0xaaff;
	note_pool_t p;
	u16 canary_after = 0xffaa;

	pool_init(&p);

	TEST_ASSERT_EQUAL_UINT16(0xaaff, canary_before);
	TEST_ASSERT_EQUAL_UINT16(0xffaa, canary_after);

	// check first element
	TEST_ASSERT_EQUAL_INT(0, p.elements[0].note.num);
	TEST_ASSERT_EQUAL_INT(0, p.elements[0].note.vel);
	TEST_ASSERT_NULL(p.elements[0].next);
	TEST_ASSERT_TRUE(p.elements[0].is_free);

	// check last element
	TEST_ASSERT_EQUAL_INT(0, p.elements[NOTE_POOL_SIZE-1].note.num);
	TEST_ASSERT_EQUAL_INT(0, p.elements[NOTE_POOL_SIZE-1].note.vel);
	TEST_ASSERT_NULL(p.elements[NOTE_POOL_SIZE-1].next);
	TEST_ASSERT_TRUE(p.elements[NOTE_POOL_SIZE-1].is_free);

	// check pool level info
	TEST_ASSERT_EQUAL_INT(0, p.count);
	TEST_ASSERT_NULL(p.head);
	TEST_ASSERT_NULL(p.last_free);
}


//
// test public
//
void test_notes_init(void) {
	// ensure initializer doesn't crash
	note_pool_t p;
	notes_init(&p);
}


int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_pool_init);
	RUN_TEST(test_notes_init);

	return UNITY_END();
}
