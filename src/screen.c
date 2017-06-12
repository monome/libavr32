// ASF
#include "board.h"
#include "delay.h"
#include "gpio.h"
#include "intc.h"
#include "print_funcs.h"
#include "spi.h"

// libavr32
#include "font.h"
#include "screen.h"
#include "interrupts.h"

//-----------------------------
//---- variables

// screen buffer
static u8 screenBuf[GRAM_BYTES];

// common temp vars
static u32 i, j;
static u8* pScr; // movable pointer to screen buf
static u32 nb; // count of destination bytes

static void write_command(U8 c);
static void write_command(U8 c) {
  u8 irq_flags = irqs_pause();
  spi_selectChip(OLED_SPI, OLED_SPI_NPCS);
  // pull register select low to write a command
  gpio_clr_gpio_pin(OLED_DC_PIN);
  spi_write(OLED_SPI, c);
  spi_unselectChip(OLED_SPI, OLED_SPI_NPCS);
  irqs_resume(irq_flags);
}

// set the current drawing area of the physical screen (hopefully)
static void screen_set_rect(u8 x, u8 y, u8 w, u8 h);
void screen_set_rect(u8 x, u8 y, u8 w, u8 h) {
 // set column address
  write_command(0x15);		// command
  write_command(x);	// column start
  write_command((x+w-1));	// column end
  // set row address
  write_command(0x75);		// command
  write_command(y);	// column start
  write_command(y+h-1);	// column end
}

//------------------
// al functions
void init_oled(void) {
  Disable_global_interrupt();
  // flip the reset pin
  gpio_set_gpio_pin(OLED_RES_PIN);
  delay_ms(1);
  gpio_clr_gpio_pin(OLED_RES_PIN);
  delay_ms(1);
  gpio_set_gpio_pin(OLED_RES_PIN);
  delay_ms(10);

  //// initialize OLED
  write_command(0xAE);	// off
  write_command(0xB3);	// clock rate
  write_command(0x91);
  write_command(0xA8);	// multiplex
  write_command(0x3F);
  write_command(0x86);	// full current range
  write_command(0x81);	// contrast to full
  write_command(0x7F);
  write_command(0xB2);	// frame freq
  write_command(0x51);
  write_command(0xA8);	// multiplex
  write_command(0x3F);
  write_command(0xBC);	// precharge
  write_command(0x10);
  write_command(0xBE);	// voltage
  write_command(0x1C);
  write_command(0xAD);	// dcdc
  write_command(0x02);
  write_command(0xA0);	// remap
  write_command(0x50);	// 0b01010000
			// a[6] : enable COM split odd/even
			// a[4] : enable COM re-map
  write_command(0xA1);	// start
  write_command(0x0);
  write_command(0xA2);	// offset
  write_command(0x4C);
  write_command(0xB1);	// set phase
  write_command(0x55);
  write_command(0xB4);	// precharge
  write_command(0x02);
  write_command(0xB0);	// precharge
  write_command(0x28);
  write_command(0xBF);	// vsl
  write_command(0x0F);
  write_command(0xA4);	// normal display
  write_command(0xB8);	// greyscale table
  write_command(0x01);
  write_command(0x11);
  write_command(0x22);
  write_command(0x32);
  write_command(0x43);
  write_command(0x54);
  write_command(0x65);
  write_command(0x76);	
		
  // set update box (to full screen)
  write_command(0x15);
  write_command(0);
  write_command(63);
  write_command(0x75);
  write_command(0);
  write_command(63); // ???

  screen_clear();
  
  write_command(0xAF);	// on
  delay_ms(10) ;
  //  cpu_irq_enable();
  Enable_global_interrupt();
}

static void writeScreenBuffer(u8 x, u8 y, u8 w, u8 h) {
  // set drawing region
  screen_set_rect(x, y, w, h);

  u8 irq_flags = irqs_pause();

  // select chip for data
  spi_selectChip(OLED_SPI, OLED_SPI_NPCS);
  // register select high for data
  gpio_set_gpio_pin(OLED_DC_PIN);
  // send data
  for(i=0; i<(nb); i++) {
    spi_write(OLED_SPI, screenBuf[i]);
  }
  spi_unselectChip(OLED_SPI, OLED_SPI_NPCS);

  irqs_resume(irq_flags);
}

// draw data given target rect
// assume x-offset and width are both even!
 void screen_draw_region(u8 x, u8 y, u8 w, u8 h, u8* data) {
  // 1 row address = 2 horizontal pixels
  // physical screen memory: 2px = 1byte
  w >>= 1;
  x >>= 1;
  nb = w * h;
  
  #ifdef MOD_ALEPH // aleph screen is mounted upside down...
  pScr = (u8*)screenBuf + nb - 1;  
  x = SCREEN_ROW_BYTES - x - w;
  y = SCREEN_COL_BYTES - y - h;
   // copy and pack into the screen buffer
  // 2 bytes input per 1 byte output
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++) {
	  *pScr = (0xf0 & ((*data) << 4) );
      data++;
      *pScr |= ((*data) & 0xf);
      data++;
      pScr--;
    }
  }
  #else 
  pScr = (u8*)screenBuf;
   // copy and pack into the screen buffer
  // 2 bytes input per 1 byte output
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++) {
      *pScr = ((*data) & 0xf);
      data++;
      *pScr |= (0xf0 & ((*data) << 4) );
      data++;
      pScr++;
    }
  }
  #endif

  writeScreenBuffer(x, y, w, h);
}

// draw data at given rectangle, with starting byte offset within the region data.
// will wrap to beginning of region
// useful for scrolling buffers
void screen_draw_region_offset(u8 x, u8 y, u8 w, u8 h, u32 len, u8* data, u32 off) {
  // store region bounds  for wrapping
  // inclusive lower bound
  u8* const dataStart = data;
  // exclusive upper bound
  u8* const dataEnd = data + len - 1;
  // begin at specified offset in region
  data += off;

  // 1 row address = 2 horizontal pixels
  // physical screen memory: 2px = 1byte
  w >>= 1;
  x >>= 1;
  nb = len >> 1;

  
  #ifdef MOD_ALEPH // aleph screen is mounted upside down...
  pScr = (u8*)screenBuf + nb - 1;  
  x = SCREEN_ROW_BYTES - x - w;
  y = SCREEN_COL_BYTES - y - h;
   // copy and pack into the screen buffer
  // 2 bytes input per 1 byte output
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++) {
	  *pScr = (0xf0 & ((*data) << 4) );
      data++;
      if(data > dataEnd) { data = dataStart; }
      *pScr |= ((*data) & 0xf);
      data++;
      if(data > dataEnd) { data = dataStart; }
      pScr--;
    }
  }
  #else 
  pScr = (u8*)screenBuf;
   // copy and pack into the screen buffer
  // 2 bytes input per 1 byte output
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++) {
      *pScr = ((*data) & 0xf);
      data++;
      if(data > dataEnd) { data = dataStart; }
      *pScr |= (0xf0 & ((*data) << 4) );
      data++;
      if(data > dataEnd) { data = dataStart; }
      pScr++;
    }
  }
  #endif
  
  writeScreenBuffer(x, y, w, h);
}


 // clear OLED RAM and local screenbuffer
void screen_clear(void) {
  u8 irq_flags = irqs_pause();
  spi_selectChip(OLED_SPI, OLED_SPI_NPCS);
  // pull register select high to write data
  gpio_set_gpio_pin(OLED_DC_PIN);
  for(i=0; i<GRAM_BYTES; i++) { 
    screenBuf[i] = 0;
    spi_write(OLED_SPI, 0);
  }
  spi_unselectChip(OLED_SPI, OLED_SPI_NPCS);
  irqs_resume(irq_flags);
}
