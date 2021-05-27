#ifndef _USB_CDC_H_
#define _USB_CDC_H_

#include <stdint.h>

#include "uhc.h"

void cdc_change(uhc_device_t* dev, uint8_t plug);

#endif
