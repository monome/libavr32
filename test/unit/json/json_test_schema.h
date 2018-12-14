#pragma once

#include <stdbool.h>

#define LONG_STRING \
	"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" \
	"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" \
	"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" \
	"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF01234567"

typedef enum {
	TEST_ENUM_ZERO,
	TEST_ENUM_ONE,
	TEST_ENUM_TWO,
} test_enum_t;

typedef struct {
	uint8_t ubyte;
} test_nested_t;

typedef struct {
	uint8_t ubyte;
	int8_t sbyte;
	uint16_t ushort;
	int16_t sshort;
	uint32_t ulong;
	int32_t slong;
	bool boolean;
	test_enum_t test_enum;
	uint8_t buffer[16];
	test_nested_t nested;
	test_nested_t nested_array[4];
	char longstring[sizeof(json_test_buf) - 8];
} json_test_dest_t;

extern json_test_dest_t json_test_dest;
extern json_docdef_t json_test_docdef;
