#ifndef _INIT_COMMON_H_
#define _INIT_COMMON_H_

#include <stdint.h>

extern void init_tc(void);
extern void init_usb_host (void);
extern void init_i2c_master(void);
extern void init_i2c_slave(uint8_t addr);

#endif
