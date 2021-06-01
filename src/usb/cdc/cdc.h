#ifndef _USB_CDC_H_
#define _USB_CDC_H_

#include <stdint.h>

#include "uhc.h"

#define UHI_CDC_RX_NOTIFY() cdc_rx_notify()

void cdc_change(uhc_device_t* dev, uint8_t plug);
void cdc_read(void);
void cdc_write(uint8_t* data, uint32_t bytes);

uint8_t cdc_connected(void);
extern uint8_t* cdc_rx_buf(void);
extern volatile uint8_t cdc_rx_bytes(void);
extern volatile uint8_t cdc_rx_busy(void);
extern volatile uint8_t cdc_tx_busy(void);

#endif
