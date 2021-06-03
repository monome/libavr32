#include "cdc.h"

// std
#include <stdint.h>

// asf
#include "uhc.h"
#include "print_funcs.h"
#include "uhi_cdc.h"

// libavr32
#include "events.h"
#include "monome.h"

#define CDC_RX_BUF_SIZE 64

//----- static vars
static uint8_t rxBuf[CDC_RX_BUF_SIZE];
static uint32_t rxBytes = 0;
static uint8_t rxBusy = 0;
static uint8_t txBusy = 0;
static uint8_t connected = 0;
static event_t e;

extern uint8_t* cdc_rx_buf() { return rxBuf; }
extern volatile uint8_t cdc_rx_bytes() { return rxBytes; }
extern volatile uint8_t cdc_rx_busy() { return rxBusy; }
extern volatile uint8_t cdc_tx_busy() { return txBusy; }
extern uint8_t cdc_connected(void) { return connected; }

void cdc_tx(void);
void cdc_rx(void);

void cdc_change(uhc_device_t* dev, uint8_t plug) {
  if(plug) {
    //print_dbg("\r\ncdc connected");
    connected = true;
    e.type = kEventSerialConnect;
  } else {
    //print_dbg("\r\ncdc disconnected");
    connected = false;
    e.type = kEventSerialDisconnect;
  }
  event_post(&e);
}

static void cdc_rx_done(usb_add_t add,
                         usb_ep_t ep,
                         uhd_trans_status_t stat,
                         iram_size_t nb) {
  rxBytes = nb;

  // FIXME: if the buffer is full, it's a false receive
  if (rxBytes != CDC_RX_BUF_SIZE) {
    if(monome_read_serial != NULL) {
      (*monome_read_serial)();
    }
    /*
    print_dbg("\r\nrx: ");
    for(int i=0;i<rxBytes;i++) {
      print_dbg_ulong(rxBuf[i]);
      print_dbg(" ");
    }*/
  }

  rxBusy = false;
}

static void cdc_tx_done(usb_add_t add,
                         usb_ep_t ep,
                         uhd_trans_status_t stat,
                         iram_size_t nb) {
  txBusy = false;

  // FIXME: bunch of these at startup
  /*if (stat != UHD_TRANS_NOERROR) {
    print_dbg("\r\ntx transfer callback error. status: 0x");
    print_dbg_hex((u32)stat);
  }*/
}



//-------- extern functions
void cdc_write(uint8_t* data, uint32_t bytes) {
  if (txBusy == false) {
    txBusy = true;
    if(!uhi_cdc_out_run(data, bytes, &cdc_tx_done)) {
      print_dbg("\r\ntx transfer error");
      txBusy = false;
    }
  }
}

void cdc_read(void) {
  if (rxBusy == false) {
    rxBytes = 0;
    rxBusy = true;
    if (!uhi_cdc_in_run((uint8_t*)rxBuf,
                         CDC_RX_BUF_SIZE, &cdc_rx_done)) {
      print_dbg("\r\n rx transfer error");
      rxBusy = false;
    }
  }
}

