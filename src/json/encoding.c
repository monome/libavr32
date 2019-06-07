#include <stdbool.h>

#include "json/encoding.h"

#define JSON_DEBUG 0
#if JSON_DEBUG
#include "print_funcs.h"
#endif

char* encode_decimal_unsigned(uint32_t val) {
	static char decimal_encoding_buf[12] = { 0 };
	uint8_t i;

#if JSON_DEBUG
	print_dbg("\r\nencode unsigned ");
	print_dbg_hex(val);
#endif
	for (i = 0; i < 12; i++) {
	  decimal_encoding_buf[i] = 0;
	}
	i = 10;
	if (val == 0) {
		decimal_encoding_buf[i--] = '0';
	}
	else {
		for (; val && i; --i, val /= 10) {
			decimal_encoding_buf[i] = (val % 10) + '0';
		}
	}
#if JSON_DEBUG
	print_dbg("-> ");
	print_dbg(&decimal_encoding_buf[i + 1]);
#endif
	return &decimal_encoding_buf[i + 1];
}

char* encode_decimal_signed(int32_t val) {
	char* ret;
	if (val < 0) {
		ret = encode_decimal_unsigned(-val);
		*--ret = '-';
	} else {
		ret = encode_decimal_unsigned(val);
	}
	return ret;
}

int32_t decode_decimal(const char* s, int len) {
	int32_t ret = 0;

#if JSON_DEBUG
	print_dbg("\r\ndecode scalar: ");
	for(int i = 0; i < len; i++) {
		print_dbg_char(s[i]);
	}
#endif

	// also handle bool
	if (s[0] == 't') {
		return 1;
	}
	if (s[0] == 'f') {
		return 0;
	}
	bool negative = s[0] == '-';
	for (int i = negative ? 1 : 0; i < len; i++) {
		ret = ret * 10 + (s[i] - '0');
	}
        if (negative) {
		ret = -ret;
	}

#if JSON_DEBUG
	print_dbg(" -> ");
	print_dbg_hex(ret);
#endif

	return ret;
}

int decode_nybble(uint8_t* dst, char hex) {
	if (hex < '0') {
		return -1;
	}
	if (hex <= '9') {
		*dst = hex - '0';
		return 0;
	}
	if (hex < 'A' || hex > 'f') {
		return -1;
	}
	if (hex >= 'a') {
		hex -= ('a' - 'A');
	}
	if (hex > 'F') {
		return -1;
	}
        *dst = hex - 'A' + 0xA;
	return 0;
}

int decode_hexbuf(json_copy_cb copy, char* dst, const char* src, size_t len) {
	uint8_t upper, lower;
	char byte;
	for (size_t i = 0; i < len; i += 2) {
		if (decode_nybble(&upper, src[i]) < 0) {
			return -1;
		}
		if (decode_nybble(&lower, src[i + 1]) < 0) {
			return -1;
		}
		byte = (upper << 4) | lower;
		copy(dst + i / 2, &byte, 1);
	}
	return 0;
}

char encode_nybble(uint8_t value) {
	if (value > 0x9) {
		return value - 0xA + 'A';
	}
	return value + '0';
}

void encode_hexbuf(json_puts_cb write, const uint8_t* src, size_t len) {
	char nybble;
	for (size_t i = 0; i < len; i++) {
		nybble = encode_nybble((src[i] & 0xF0) >> 4);
		write(&nybble, 1);
		nybble = encode_nybble(src[i] & 0x0F);
		write(&nybble, 1);
	}
}
