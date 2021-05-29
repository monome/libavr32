#include "cdc.h"

// std
#include <stdint.h>

// asf
#include "uhc.h"
#include "print_funcs.h"
#include "uhi_cdc.h"

#include "gpio.h"

// libavr32
//#include "events.h"


#define CDC_COM_PORT 0

static volatile bool flag_cdc_available = false;

static volatile uint8_t rxBuf[64];

void cdc_tx(void);
void cdc_rx(void);

void cdc_change(uhc_device_t* dev, uint8_t plug) {
  print_dbg("\r\ncdc plug ");

  
  if(plug) {
    // USB Device CDC connected
    flag_cdc_available = true;

    // Open and configure USB CDC ports
    usb_cdc_line_coding_t cfg = {
      .dwDTERate   = CPU_TO_LE32(115200),
      .bCharFormat = CDC_STOP_BITS_1,
      .bParityType = CDC_PAR_NONE,
      .bDataBits   = 8,
    };
  switch(dev->speed) {      
    case UHD_SPEED_HIGH:
        print_dbg("\r\nconnecting; high speed");
        break;
    case UHD_SPEED_FULL:
        print_dbg("\r\nconnecting; full speed");
        break;
    case UHD_SPEED_LOW:
    default:
        print_dbg("\r\nconnecting; low speed");
        break;
    }
    
    bool success = uhi_cdc_open(CDC_COM_PORT, &cfg);
    
    if (success) {    
      print_dbg("\r\ncdc connected");

      print_dbg("\r\n  dwDTERate: ");
      print_dbg_ulong(LE32_TO_CPU(cfg.dwDTERate));
      print_dbg("\r\n  bCharFormat: ");
      print_dbg_ulong(cfg.bCharFormat);
      print_dbg("\r\  nbParityType: ");
      print_dbg_ulong(cfg.bParityType);
      print_dbg("\r\n  bDataBits: ");
      print_dbg_ulong(cfg.bDataBits);

      
    } else {
	    print_dbg("\r\ncdc failed to connect");
    }

  } else {
    flag_cdc_available = false;
  }
}


void cdc_rx_notify(void)
{
  //print_dbg("\r\ncdc_rx_notify");
  if (flag_cdc_available) {
  //cdc_tx();
    cdc_rx();
  }
}

void cdc_tx(void)
{
  //print_dbg("\r\ntx\r\n");
  //uhi_cdc_write_buf(0, MESSAGE, sizeof(MESSAGE)-1);
  //uhi_cdc_putc(0,'\n');
  // https://github.com/trentgill/gridST/blob/main/main.c#L123
  uhi_cdc_putc(0,0x18); // set all to level
  uhi_cdc_putc(0,0x0F); // full bright
}

void cdc_rx(void)
{
  iram_size_t nb = uhi_cdc_get_nb_received(CDC_COM_PORT);
  if (nb > 0) {
    uhi_cdc_read_buf(CDC_COM_PORT, (void*)rxBuf, nb);
    if (nb == 64) { 
      /// FIXME: why is this always happening?
      /// TODO: next debug step would be to save a copy of the full 64B buffer,
	/// and check for differences
      gpio_clr_gpio_pin(B00);
      gpio_clr_gpio_pin(B01);
    } else { 
      print_dbg("\r\ncdc rx:\r\n");
      for (int i=0; i<nb; ++i) {
        uint8_t value = rxBuf[i];
        print_dbg_hex(value);#include "cdc.h"

// std
#include <stdint.h>

// asf
#include "uhc.h"
#include "print_funcs.h"
#include "uhi_cdc.h"

#include "gpio.h"

// libavr32
//#include "events.h"


#define CDC_COM_PORT 0

static volatile bool flag_cdc_available = false;

static volatile uint8_t rxBuf[64];

void cdc_tx(void);
void cdc_rx(void);

void cdc_change(uhc_device_t* dev, uint8_t plug) {
  print_dbg("\r\ncdc plug ");

  
  if(plug) {
    // USB Device CDC connected
    flag_cdc_available = true;

    // Open and configure USB CDC ports
    usb_cdc_line_coding_t cfg = {
      .dwDTERate   = CPU_TO_LE32(115200),
      .bCharFormat = CDC_STOP_BITS_1,
      .bParityType = CDC_PAR_NONE,
      .bDataBits   = 8,
    };
  switch(dev->speed) {      
    case UHD_SPEED_HIGH:
        print_dbg("\r\nconnecting; high speed");
        break;
    case UHD_SPEED_FULL:
        print_dbg("\r\nconnecting; full speed");
        break;
    case UHD_SPEED_LOW:
    default:
        print_dbg("\r\nconnecting; low speed");
        break;
    }
    
    bool success = uhi_cdc_open(CDC_COM_PORT, &cfg);
    
    if (success) {    
      print_dbg("\r\ncdc connected");

      print_dbg("\r\n  dwDTERate: ");
      print_dbg_ulong(LE32_TO_CPU(cfg.dwDTERate));
      print_dbg("\r\n  bCharFormat: ");
      print_dbg_ulong(cfg.bCharFormat);
      print_dbg("\r\  nbParityType: ");
      print_dbg_ulong(cfg.bParityType);
      print_dbg("\r\n  bDataBits: ");
      print_dbg_ulong(cfg.bDataBits);

      
    } else {
	    print_dbg("\r\ncdc failed to connect");
    }

  } else {
    flag_cdc_available = false;
  }
}


void cdc_rx_notify(void)
{
  //print_dbg("\r\ncdc_rx_notify");
  if (flag_cdc_available) {
  //cdc_tx();
    cdc_rx();
  }
}

void cdc_tx(void)
{
  //print_dbg("\r\ntx\r\n");
  //uhi_cdc_write_buf(0, MESSAGE, sizeof(MESSAGE)-1);
  //uhi_cdc_putc(0,'\n');
  // https://github.com/trentgill/gridST/blob/main/main.c#L123
  uhi_cdc_putc(0,0x18); // set all to level
  uhi_cdc_putc(0,0x0F); // full bright
}

void cdc_rx(void)
{
  iram_size_t nb = uhi_cdc_get_nb_received(CDC_COM_PORT);
  if (nb > 0) {
    uhi_cdc_read_buf(CDC_COM_PORT, (void*)rxBuf, nb);
    if (nb == 64) { 
      /// FIXME: why is this always happening?
      /// TODO: next debug step could be to save a copy of the full 64B buffer,
	/// and check for differences
      gpio_clr_gpio_pin(B00);
      gpio_clr_gpio_pin(B01);
    } else { 
      print_dbg("\r\ncdc rx:\r\n");
      for (int i=0; i<nb; ++i) {
        uint8_t value = rxBuf[i];
        print_dbg_hex(value);
	print_dbg(" ");
      }
      gpio_set_gpio_pin(B00);
      gpio_set_gpio_pin(B01);
    }
  } else {
    // zero bytes read
    gpio_set_gpio_pin(B00);
    gpio_clr_gpio_pin(B01);
  }
}

      }
      gpio_set_gpio_pin(B00);
      gpio_set_gpio_pin(B01);
    }
  } else {
    // zero bytes read
    gpio_set_gpio_pin(B00);
    gpio_clr_gpio_pin(B01);
  }
}
