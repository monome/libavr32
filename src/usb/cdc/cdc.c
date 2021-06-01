#include "cdc.h"

// std
#include <stdint.h>

// asf
#include "uhc.h"
#include "print_funcs.h"
#include "uhi_cdc.h"

// libavr32
//#include "events.h"

static bool flag_cdc_available = false;

void cdc_tx(void);
void cdc_rx(void);

void cdc_change(uhc_device_t* dev, uint8_t plug) {
  print_dbg("\r\ncdc plug ");

  if(plug) {
    // USB Device CDC connected
    flag_cdc_available = true;
    // Open and configure USB CDC ports
/*    usb_cdc_line_coding_t cfg = {
      .dwDTERate   = CPU_TO_LE32(115200),
      .bCharFormat = CDC_STOP_BITS_1,
      .bParityType = CDC_PAR_NONE,
      .bDataBits   = 8,
    };
    uhi_cdc_open(0, &cfg);*/
    //uhi_cdc_open(0);

    print_dbg("\r\ncdc connected");

  } else {

    flag_cdc_available = false;

  }
}


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

