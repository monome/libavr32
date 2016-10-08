#ifndef _ADC_H_
#define _ADC_H_

#include "compiler.h"
#include "types.h"

// setup ad7923
extern void init_adc(void);

extern void adc_convert(u16 (*dst)[4]);

#endif

