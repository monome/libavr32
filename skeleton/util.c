#include "types.h"
#include "util.h"

u32 rnd(void) {
  x1 = x1 * c1 + a1;
  x2 = x2 * c2 + a2;
  return (x1>>16) | (x2>>16);
}


u16 rotl(u16 value, u16 shift) {
    return (value << shift) | (value >> (16 - shift));
}