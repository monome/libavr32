#include "cdc.h"

// std
#include <stdint.h>

// asf
#include "uhc.h"
#include "print_funcs.h"
#include "uhi_cdc.h"

// libavr32
#include "events.h"

#define CDC_RX_BUF_SIZE 64

//----- static vars
static uint8_t rxBuf[CDC_RX_BUF_SIZE];
static uint32_t rxBytes = 0;
static uint8_t rxBusy = 0;
static uint8_t txBusy = 0;
static event_t e;

static bool connected = false;


bool cdc_connected(void) {
  return connected;
}

void cdc_tx(void);
void cdc_rx(void);

void cdc_change(uhc_device_t* dev, uint8_t plug) {
  if(plug) {
    connected = true;
    print_dbg("\r\ncdc connected");
  } else {
    connected = false;
    print_dbg("\r\ncdc disconnected ");
  }
}

static void cdc_rx_done(usb_add_t add,
                         usb_ep_t ep,
                         uhd_trans_status_t stat,
                         iram_size_t nb) {
  //rxBytes = nb - FTDI_STATUS_BYTES;
  rxBytes = nb;

  /* if (stat != UHD_TRANS_NOERROR) { */
  /*   print_dbg("\r\n ftdi rx transfer callback error. status: 0x"); */
  /*   print_dbg_hex((u32)stat); */
  /*   print_dbg(" ; bytes transferred: "); */
  /*   print_dbg_ulong(nb); */
  /*   print_dbg(" ; status bytes: 0x"); */
  /*   print_dbg_hex(rxBuf[0]); */
  /*   print_dbg(" 0x"); */
  /*   print_dbg_hex(rxBuf[1]); */
  /* } */
  /* else */

  if (rxBytes) {
    // check for monome events
    //    if(monome_read_serial != NULL) {
    for(int i=0;i<rxBytes;i++) {
      print_dbg_ulong(rxBuf[i]);
      print_dbg(" ");
    }
    //(*monome_read_serial)();
    //}
    ///... TODO: other protocols
  }

  print_dbg("\r\n");

  rxBusy = false;
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





/*
void cdc_rx_notify(void)
{
  print_dbg("\r\ncdc_rx_notify");
  cdc_tx();
  cdc_rx();
}

void cdc_tx(void)
{
  print_dbg("\r\ntx\r\n");
  //uhi_cdc_write_buf(0, MESSAGE, sizeof(MESSAGE)-1);
  //uhi_cdc_putc(0,'\n');
  // https://github.com/trentgill/gridST/blob/main/main.c#L123
  //uhi_cdc_putc(0,0x18); // set all to level
  //uhi_cdc_putc(0,0x0F); // full bright
}

void cdc_rx(void)
{
  //while (uhi_cdc_is_rx_ready(0)) {
  //int value = uhi_cdc_getc(0);
  //print_dbg_ulong(value);
  //print_dbg_char(' ');
  //}
}
*/
