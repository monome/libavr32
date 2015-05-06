#include "print_funcs.h"
#include "twi.h"
#include "events.h"
#include "i2c.h"

volatile u8 i2c_buffer[I2C_BUFFER_SIZE];

static u16 i2c_read_pos = 0;
static u16 i2c_write_pos = 0;

twi_package_t packet_tx = { 
	.chip = 102,
  //  .addr = {100, 100, 100}, 
	.addr_length=0,
	.length=1 
}; 

twi_package_t packet_rx = { 
	.chip = 100, 
  //  .addr = {100, 100, 100}, 
  //  .addr_length=0,
	.length=1 
}; 


// transmit data in slave mode
// __attribute__((__interrupt__))
u8 i2c_slave_tx(void) {
	u8 val = 0;
	return val;
}

// handler for rx events
//__attribute__((__interrupt__))
void i2c_slave_rx(u8 value) {
  // print_dbg("\r\n slave rx: ");
  // print_dbg_char_hex(value);

	i2c_buffer[i2c_write_pos] = value;
	i2c_write_pos++;
	if(i2c_write_pos == I2C_BUFFER_SIZE) i2c_write_pos = 0;
}

// stop function - done receiving??
void i2c_slave_stop(void) {
	// static event_t e;   
	// e.type = kEventI2C;
	// e.data = i2c_read_pos;
	// event_post(&e);

	i2c_read_pos = i2c_write_pos;
}

// master send
void i2c_master_tx(u8* tx) {
	print_dbg("\r\n i2c_master_tx, data location: 0x");
	print_dbg_hex((u32)tx);
	print_dbg(", data value: 0x");
	print_dbg_char_hex(*tx);
	packet_tx.buffer = tx;

	twi_master_write(&AVR32_TWI, &packet_tx);
	print_dbg("\r\ndone.");
}

// master receive
void i2c_master_rx(u8* rx) {
	packet_rx.buffer = rx;
	twi_master_read(&AVR32_TWI, &packet_rx);
}
