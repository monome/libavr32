#ifndef _DAC_H_
#define _DAC_H_

#include "compiler.h"

#define DAC_RATE_CV 6
#define DAC_10V 16383

void init_dacs(void);
void reset_dacs(void);

void update_dacs(uint16_t *d);

void dac_set_value_noslew(uint8_t n, uint16_t v);
void dac_set_value(uint8_t n, uint16_t v);
void dac_set_slew(uint8_t n, uint16_t s);
void dac_set_off(uint8_t n, int16_t o);

void dac_timer_update(void);

bool dac_is_slewing(uint8_t n);

#endif