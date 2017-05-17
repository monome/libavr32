#ifndef _USB_MSC_H_
#define _USB_MSC_H_

#include <stdint.h>

#include "uhc.h"

void msc_change(uhc_device_t* dev, uint8_t plug);

#endif
