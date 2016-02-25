#ifndef _INIT_TELETYPE_H_
#define _INIT_TELETYPE_H_

#include "types.h"

// global count of uptime, and overflow flag.
volatile u64 tcTicks;
volatile u8 tcOverflow;

extern void register_interrupts(void);
extern void init_gpio(void);
extern void init_spi(void);

#endif
