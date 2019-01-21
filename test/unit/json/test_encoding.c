#include <string.h>

#include "unity.h"

#include "json_test_common.c"

void test_decimal_signed_round_trip(void) {
	{
		char s[] = "0";
		TEST_ASSERT_EQUAL_INT32(0, decode_decimal(s, strlen(s)));
		char* dec = encode_decimal_signed(0);
		TEST_ASSERT_EQUAL_STRING(s, dec);
	}
	{
		char s[] = "123";
		TEST_ASSERT_EQUAL_INT32(123, decode_decimal(s, strlen(s)));
		char* dec = encode_decimal_signed(123);
		TEST_ASSERT_EQUAL_STRING(s, dec);
	}
	{
		char s[] = "2147483647";
		TEST_ASSERT_EQUAL_INT32(2147483647, decode_decimal(s, strlen(s)));
		char* dec = encode_decimal_signed(2147483647);
		TEST_ASSERT_EQUAL_STRING(s, dec);
	}
	{
		char s[] = "-123";
		TEST_ASSERT_EQUAL_INT32(-123, decode_decimal(s, strlen(s)));
		char* dec = encode_decimal_signed(-123);
		TEST_ASSERT_EQUAL_STRING(s, dec);
	}
	{
		char s[] = "-2147483648";
		TEST_ASSERT_EQUAL_INT32(-2147483647 - 1, decode_decimal(s, strlen(s)));
		char* dec = encode_decimal_signed(-2147483647 - 1);
		TEST_ASSERT_EQUAL_STRING(s, dec);
	}
}

void test_decimal_unsigned_round_trip(void) {
	{
		char s[] = "0";
		TEST_ASSERT_EQUAL_UINT32(0, decode_decimal(s, strlen(s)));
		char* dec = encode_decimal_unsigned(0);
		TEST_ASSERT_EQUAL_STRING(s, dec);
	}
	{
		char s[] = "4294967295";
		TEST_ASSERT_EQUAL_UINT32(4294967295, decode_decimal(s, strlen(s)));
		char* dec = encode_decimal_unsigned(4294967295);
		TEST_ASSERT_EQUAL_STRING(s, dec);
	}
}

void test_decode_hexbuf(void) {
	char hex_out[9] = { 0 };
	{
		char s[] = "00112233";
		TEST_ASSERT_EQUAL_INT(0, decode_hexbuf(copy, hex_out, s, strlen(s)));
		TEST_ASSERT_EQUAL_STRING(hex_out, "\x00\x11\x22\x33");
	}
	{
		char s[] = "AABBCCDD";
		TEST_ASSERT_EQUAL_INT(0, decode_hexbuf(copy, hex_out, s, strlen(s)));
		TEST_ASSERT_EQUAL_STRING(hex_out, "\xAA\xBB\xCC\xDD");
	}
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_decimal_signed_round_trip);
	RUN_TEST(test_decimal_unsigned_round_trip);
	RUN_TEST(test_decode_hexbuf);

	return UNITY_END();
}
