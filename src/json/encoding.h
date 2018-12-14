#pragma once

char* encode_decimal_unsigned(uint32_t val);
char* encode_decimal_signed(int32_t val);
int32_t decode_decimal(const char* s, int len);
int decode_hexbuf(char* dst, const char* src, size_t len);
char encode_nybble(uint8_t val);
