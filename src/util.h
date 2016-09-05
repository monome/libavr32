#ifndef _UTIL_H_
#define _UTIL_H_

#include "types.h"

static const u32 a1 = 0x19660d;
static const u32 c1 = 0x3c6ef35f;
static volatile u32 x1 = 1234567;  // seed
static const u32 a2 = 0x19660d;
static const u32 c2 = 0x3c6ef35f;
static volatile u32 x2 = 7654321;  // seed

extern u32 rnd(void);
extern u16 rotl(u16 value, u16 shift);
char* itoa(int value, char* result, int base);
extern u32 uclip(u32 value, u32 low, u32 high);

#endif
