#ifndef _USB_CDC_H_
#define _USB_CDC_H_

#include <stdint.h>

#include "uhc.h"

#define UHI_CDC_RX_NOTIFY() cdc_rx_notify()

void cdc_change(uhc_device_t* dev, uint8_t plug);
void cdc_rx_notify(void);

#endif
