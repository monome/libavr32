#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "types.h"

u8 irqs_pause(void);
void irqs_resume(u8 irq_flags);

#endif
