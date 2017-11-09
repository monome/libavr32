#ifndef _INIT_TELETYPE_H_
#define _INIT_TELETYPE_H_

#include "types.h"

extern void register_interrupts(void);
extern void init_gpio(void);
extern void init_spi(void);

extern u64 get_ticks(void);

#endif
