#include <stdint.h>
#include <string.h>

#include "unity.h"

#include "json_test_common.c"
#include "json_test_schema.c"

void test_all_types_round_trip(void) {
	const char s[] = "{"
		"\"ubyte\": 0, "
		"\"sbyte\": -127, "
		"\"ushort\": 32768, "
		"\"sshort\": -32767, "
		"\"ulong\": 4294967295, "
		"\"slong\": -2147483648, "
		"\"boolean\": true, "
		"\"test_enum\": \"TWO\", "
		"\"buffer\": \"00010203040506070809A0B0C0D0E0F0\", "
		"\"nested\": {"
			"\"ubyte\": 255"
		"}, "
		"\"nested_array\": ["
			"{"
				"\"ubyte\": 1"
			"}, "
			"{"
				"\"ubyte\": 2"
			"}, "
			"{"
				"\"ubyte\": 3"
			"}, "
			"{"
				"\"ubyte\": 4"
			"}"
		"], "
		"\"nested_cached\": {"
			"\"ubyte\": 127"
		"}, "
		"\"longstring\": \""
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF01234567"
	  	"\", "
		"\"longbuffer\": \""
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
			"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF01234567"
		"\""
	"}";
	FILE* fp = write_temp_file("in___all_types_round_trip.tmp", s, strlen(s));
	set_fp(fp);
	memset(&json_test_dest, 0, sizeof(json_test_dest_t));

	json_read_result_t rd_result = json_read(
		read_fp, copy,
		&json_test_dest, &json_test_docdef,
		json_test_buf, sizeof(json_test_buf),
		json_test_tokens, sizeof(json_test_tokens) / sizeof(json_test_tokens[0]));
	fclose(fp);

	TEST_ASSERT_EQUAL_INT(rd_result, JSON_READ_OK);
	TEST_ASSERT_EQUAL_UINT8(0, json_test_dest.ubyte);
	TEST_ASSERT_EQUAL_INT8(-127, json_test_dest.sbyte);
	TEST_ASSERT_EQUAL_UINT16(32768, json_test_dest.ushort);
	TEST_ASSERT_EQUAL_INT16(-32767, json_test_dest.sshort);
	TEST_ASSERT_EQUAL_UINT32(4294967295, json_test_dest.ulong);
	TEST_ASSERT_EQUAL_INT32(-2147483647 - 1, json_test_dest.slong);
	TEST_ASSERT_EQUAL_INT(true, json_test_dest.boolean);
	TEST_ASSERT_EQUAL_INT(TEST_ENUM_TWO, json_test_dest.test_enum);
	TEST_ASSERT_EQUAL_MEMORY(
		"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\xA0\xB0\xC0\xD0\xE0\xF0",
		json_test_dest.buffer,
		16);
	TEST_ASSERT_EQUAL_UINT8(255, json_test_dest.nested.ubyte);
	TEST_ASSERT_EQUAL_UINT8(127, json_test_dest.nested_cached.ubyte);
	TEST_ASSERT_EQUAL_UINT8(1, json_test_dest.nested_array[0].ubyte);
	TEST_ASSERT_EQUAL_UINT8(2, json_test_dest.nested_array[1].ubyte);
	TEST_ASSERT_EQUAL_UINT8(3, json_test_dest.nested_array[2].ubyte);
	TEST_ASSERT_EQUAL_UINT8(4, json_test_dest.nested_array[3].ubyte);
	TEST_ASSERT_EQUAL_MEMORY(LONG_STRING, json_test_dest.longstring, sizeof(LONG_STRING) - 1);

	fp = fopen("out___all_types_round_trip.tmp", "w");
	set_fp(fp);
	json_write_result_t wr_result = json_write(
		write_fp, &json_test_dest, &json_test_docdef);
	fclose(fp);

	TEST_ASSERT_EQUAL_INT(wr_result, JSON_WRITE_OK);
	TEST_ASSERT_TRUE(compare_files("in___all_types_round_trip.tmp", "out___all_types_round_trip.tmp"));
}

void test_missing_fields_ok(void) {
	const char s[] = "{"
		"\"ubyte\": 123, "
		"\"nested\": {}, "
		"\"nested_array\": ["
			"{"
				"\"ubyte\": 1"
			"}, "
			"{}, "
			"{"
				"\"ubyte\": 3"
			"}, "
			"{}"
		"]"
	"}";
	FILE* fp = write_temp_file("in___missing_fields_ok.tmp", s, strlen(s));
	set_fp(fp);
	memset(&json_test_dest, 0, sizeof(json_test_dest_t));

	json_read_result_t rd_result = json_read(
		read_fp, copy,
		&json_test_dest, &json_test_docdef,
		json_test_buf, sizeof(json_test_buf),
		json_test_tokens, sizeof(json_test_tokens) / sizeof(json_test_tokens[0]));
	fclose(fp);

	TEST_ASSERT_EQUAL_INT(rd_result, JSON_READ_OK);
	TEST_ASSERT_EQUAL_UINT8(123, json_test_dest.ubyte);
	TEST_ASSERT_EQUAL_UINT8(1, json_test_dest.nested_array[0].ubyte);
	TEST_ASSERT_EQUAL_UINT8(3, json_test_dest.nested_array[2].ubyte);
}

void test_skips_unknown_fields(void) {
	const char s[] = "{"
		"                                                                "
		"        "
		"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
		"                                "
		"                                                                "
		"        "
		"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
		"                                "
		"                                                                "
		"        "
		"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
		"                                "
		"                                                                "
		"        "
		"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
		"                                "
		"                                                                "
		"        "
		"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
		"                                "
		"                                                                "
		"        "
		"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
		"                                "
		"\"ubyte\": 234, "
		"\"unknown\": ["
			"{"
				"\"one\": 1, "
				"\"two\": 2, "
			"}, "
			"                                                                "
			"        "
			"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
			"                                "
			"                                                                "
			"        "
			"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
			"                                "
			"                                                                "
			"        "
			"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
			"                                "
			"{"
				"                                                                "
				"        "
				"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
				"                                "
				"                                                                "
				"        "
				"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
				"                                "
				"                                                                "
				"        "
				"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
				"                                "
				"\"one\": 1, "
				"\"two\": 2 "
			"}"
		"], "
		"\"sbyte\": -123"
	"}"
	"                                                                "
	"        "
	"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
	"                                "
	"                                                                "
	"        "
	"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
	"                                "
	"                                                                "
	"        "
	"\r\r\r\r\r\r\r\r\n\n\n\n\n\n\n\n\t\t\t\t\t\t\t\t"
	"                                ";
	FILE* fp = write_temp_file("in___skips_unknown_fields.tmp", s, strlen(s));
	set_fp(fp);
	memset(&json_test_dest, 0, sizeof(json_test_dest_t));

	json_read_result_t rd_result = json_read(
		read_fp, copy,
		&json_test_dest, &json_test_docdef,
		json_test_buf, sizeof(json_test_buf),
		json_test_tokens, sizeof(json_test_tokens) / sizeof(json_test_tokens[0]));
	fclose(fp);

	TEST_ASSERT_EQUAL_INT(rd_result, JSON_READ_OK);
	TEST_ASSERT_EQUAL_UINT8(234, json_test_dest.ubyte);
	TEST_ASSERT_EQUAL_INT8(-123, json_test_dest.sbyte);
}

void test_truncates_array(void) {
	const char s[] = "{"
		"\"nested_array\": ["
			"{\"ubyte\": 1}, "
			"{\"ubyte\": 2}, "
			"{\"ubyte\": 3}, "
			"{\"ubyte\": 4}, "
			"{\"ubyte\": 5}, "
			"{\"ubyte\": 6}, "
			"{\"ubyte\": 7}"
		"]"
	"}";
	FILE* fp = write_temp_file("in___truncates_array.tmp", s, strlen(s));
	set_fp(fp);
	memset(&json_test_dest, 0, sizeof(json_test_dest_t));

	json_read_result_t rd_result = json_read(
		read_fp, copy,
		&json_test_dest, &json_test_docdef,
		json_test_buf, sizeof(json_test_buf),
		json_test_tokens, sizeof(json_test_tokens) / sizeof(json_test_tokens[0]));
	fclose(fp);

	TEST_ASSERT_EQUAL_INT(rd_result, JSON_READ_OK);
	TEST_ASSERT_EQUAL_UINT8(1, json_test_dest.nested_array[0].ubyte);
	TEST_ASSERT_EQUAL_UINT8(2, json_test_dest.nested_array[1].ubyte);
	TEST_ASSERT_EQUAL_UINT8(3, json_test_dest.nested_array[2].ubyte);
	TEST_ASSERT_EQUAL_UINT8(4, json_test_dest.nested_array[3].ubyte);
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_all_types_round_trip);
	RUN_TEST(test_missing_fields_ok);
	RUN_TEST(test_skips_unknown_fields);
	RUN_TEST(test_truncates_array);

	return UNITY_END();
}
