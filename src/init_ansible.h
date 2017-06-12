#ifndef _INIT_TRILOGY_H_
#define _INIT_TRILOGY_H_

#include "types.h"

// global count of uptime, and overflow flag.
volatile u64 tcTicks;
volatile u8 tcOverflow;

// extern volatile u8 clock_external;

// typedef void(*clock_pulse_t)(u8 phase);
// extern volatile clock_pulse_t clock_pulse;

extern void register_interrupts(void);
extern void init_gpio(void);
extern void init_spi(void);

extern u64 get_ticks(void);

#endif
