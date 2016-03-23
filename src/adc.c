//ASF
#include "compiler.h"
#include "delay.h"
#include "interrupt.h"
#include "print_funcs.h"
#include "spi.h"

#include "conf_board.h"
#include "events.h"
#include "types.h"
#include "adc.h"

// ad7923 control register
#define AD7923_CTL_WRITE  (1 << 11)
#define AD7923_CTL_SEQ1   (1 << 10)
#define AD7923_CTL_ADD1   (1 << 7)
#define AD7923_CTL_ADD0   (1 << 6)
#define AD7923_CTL_PM1    (1 << 5)
#define AD7923_CTL_PM0    (1 << 4)
#define AD7923_CTL_SEQ0   (1 << 3)
#define AD7923_CTL_RANGE  (1 << 1)
#define AD7923_CTL_CODING (1)

// command word to which we add the desired channel address for conversion
#ifdef MOD_TRILOGY
#define AD7923_CMD_BASE ( AD7923_CTL_WRITE | AD7923_CTL_PM0 | AD7923_CTL_PM1 | AD7923_CTL_CODING)
#endif
#ifdef MOD_TELETYPE
#define AD7923_CMD_BASE ( AD7923_CTL_WRITE | AD7923_CTL_PM0 | AD7923_CTL_PM1 | AD7923_CTL_CODING | AD7923_CTL_RANGE)
#endif


// // adc events
// static const etype adctypes[4] = { 
//   kEventAdc0 ,
//   kEventAdc1 ,
//   kEventAdc2 ,
//   kEventAdc3 ,
// };

// perform a conversion on all 4 channels
void adc_convert(U16 (*dst)[4]) {
  U16 cmd, val;

  // data into AD7923 is a left-justified 12-bit value in a 16-bit word
  // so, always lshift the command before sending
  cmd = ( AD7923_CMD_BASE ) << 4;
  spi_selectChip(SPI, ADC_SPI);
  spi_write(SPI, cmd);
  spi_unselectChip(SPI, ADC_SPI);

  // get channel 0, setup channel 1
  cmd = ( AD7923_CMD_BASE | AD7923_CTL_ADD0 ) << 4;
  spi_selectChip(SPI, ADC_SPI);
  spi_write(SPI, cmd);
  spi_read(SPI, &val);
  spi_unselectChip(SPI, ADC_SPI);
  (*dst)[0] = val & 0xfff; 

  // get channel 1, setup channel 2
  cmd = ( AD7923_CMD_BASE | AD7923_CTL_ADD1 ) << 4;
  spi_selectChip(SPI, ADC_SPI);
  spi_write(SPI, cmd);
  spi_read(SPI, &val);
  spi_unselectChip(SPI, ADC_SPI);
  (*dst)[1] = val & 0xfff;

  // get channel 2, setup channel 3
  cmd = ( AD7923_CMD_BASE | AD7923_CTL_ADD1 | AD7923_CTL_ADD0 ) << 4;
  spi_selectChip(SPI, ADC_SPI);
  spi_write(SPI, cmd);
  spi_read(SPI, &val);
  spi_unselectChip(SPI, ADC_SPI);
  (*dst)[2] = val & 0xfff;

  // get channel 3, dummy write
  cmd = ( AD7923_CMD_BASE ) << 4;
  spi_selectChip(SPI, ADC_SPI);
  spi_write(SPI, cmd);
  spi_read(SPI, &val);
  spi_unselectChip(SPI, ADC_SPI);
  (*dst)[3] = val & 0xfff;

}

// setup ad7923
void init_adc(void) {
  u16 cmd;

  // at powerup, the part wants a dummy conversion with DIN high
  spi_selectChip(SPI, ADC_SPI);
  spi_write(SPI, 0xffff);
  spi_unselectChip(SPI, ADC_SPI);

  // wait for powerup time (5us in datasheet)
  delay_us(5);
  
  // write base configuration
  cmd = AD7923_CMD_BASE << 4;
  spi_selectChip(SPI, ADC_SPI );
  spi_write(SPI, cmd );
  spi_unselectChip(SPI, ADC_SPI );

}

// // perform conversion, check for changes, and post events
// u16 adc_poll(void) {
//   static u16 adcVal[4] = {0, 0, 0, 0};
//   // static event_t e;
//   // u8 i;

//   adc_convert(&adcVal);

//   // print_dbg("\r\nadc:\t");
//   // print_dbg_ulong(adcVal[0]); print_dbg("\t");
//   // print_dbg_ulong(adcVal[1]); print_dbg("\t");
//   // print_dbg_ulong(adcVal[2]); print_dbg("\t");
//   // print_dbg_ulong(adcVal[3]);


//   /// test with no filtering.. maybe this is fine
//   // for(i=0; i<4; i++) {        
//   //   e.type = adctypes[i];
//   //   e.data = (S16)(adcVal[i]);
//   //   event_post(&e);
//   // }

//   return adcVal[0];
// }
