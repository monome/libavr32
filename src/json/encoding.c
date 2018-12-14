#include <stdbool.h>

#include "json/encoding.h"

char* encode_decimal_unsigned(uint32_t val) {
	static char decimal_encoding_buf[12] = { 0 };
	uint8_t i = 10;
	if (val == 0) {
		decimal_encoding_buf[i--] = '0';
	}
	else {
		for (; val && i; --i, val /= 10) {
			decimal_encoding_buf[i] = (val % 10) + '0';
		}
	}
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
	return negative ? -ret : ret;
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

int decode_hexbuf(char* dst, const char* src, size_t len) {
	uint8_t upper, lower;
	for (size_t i = 0; i < len; i += 2) {
		if (decode_nybble(&upper, src[i]) < 0) {
			return -1;
		}
		if (decode_nybble(&lower, src[i + 1]) < 0) {
			return -1;
		}
		dst[i / 2] = (upper << 4) | lower;
	}
	return 0;
}

char encode_nybble(uint8_t value) {
	if (value > 0x9) {
		return value - 0xA + 'A';
	}
	return value + '0';
}
