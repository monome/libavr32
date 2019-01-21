#include <stdint.h>
#include <stddef.h>

#include "json/serdes.h"

char* encode_decimal_unsigned(uint32_t val);
char* encode_decimal_signed(int32_t val);
int32_t decode_decimal(const char* s, int len);
int decode_hexbuf(json_copy_cb copy, char* dst, const char* src, size_t len);
int decode_nybble(uint8_t* dst, char hex);
char encode_nybble(uint8_t val);
