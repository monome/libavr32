#include "spi.h"

#include "dac.h"

#include "conf_board.h"


void init_dacs(void) {
	// setup daisy chain for two dacs
	spi_selectChip(SPI,DAC_SPI);
	spi_write(SPI,0x80);
	spi_write(SPI,0xff);
	spi_write(SPI,0xff);
	spi_unselectChip(SPI,DAC_SPI);
}



void update_dacs(uint16_t *d) {
	spi_selectChip(SPI,DAC_SPI);
	spi_write(SPI,0x31);
	spi_write(SPI,d[2]>>4); // 2
	spi_write(SPI,d[2]<<4);
	spi_write(SPI,0x31);
	spi_write(SPI,d[0]>>4); // 0
	spi_write(SPI,d[0]<<4);
	spi_unselectChip(SPI,DAC_SPI);
	
	spi_selectChip(SPI,DAC_SPI);
	spi_write(SPI,0x38);
	spi_write(SPI,d[3]>>4); // 3
	spi_write(SPI,d[3]<<4);
	spi_write(SPI,0x38);
	spi_write(SPI,d[1]>>4); // 1
	spi_write(SPI,d[1]<<4);
	spi_unselectChip(SPI,DAC_SPI);
}