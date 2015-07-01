#include "print_funcs.h"
#include "twi.h"
#include "events.h"
#include "i2c.h"

static twi_package_t packet;

void i2c_master_tx(uint8_t addr, uint8_t idx, int data) {
	int status;
  uint8_t d[3];

  d[0] = idx;
  d[1] = data >> 8;
  d[2] = data & 0xff;


	  // TWI chip address to communicate with
  // packet.chip = EEPROM_ADDRESS;
  packet.chip = addr;
  // TWI address/commands to issue to the other chip (node)
  // packet.addr[0] = VIRTUALMEM_ADDR_START >> 16;
  // packet.addr[1] = VIRTUALMEM_ADDR_START >> 8;
  // packet.addr[2] = VIRTUALMEM_ADDR_START;
  // Length of the TWI data address segment (1-3 bytes)
  // packet.addr_length = EEPROM_ADDR_LGT;
  packet.addr_length = 0;
  // Where to find the data to be written
  packet.buffer = &d;
  // How many bytes do we want to write
  packet.length = 3;

  // perform a write access
  status = twi_master_write(TWI, &packet);

  // // check write result
  // if (status == TWI_SUCCESS)
  //   print_dbg("\r\nWrite test:\tPASS");
  // else
  //   print_dbg("\r\nWrite test:\tFAIL");


/*
  // TWI chip address to communicate with
  packet_received.chip = EEPROM_ADDRESS ;
  // Length of the TWI data address segment (1-3 bytes)
  packet_received.addr_length = EEPROM_ADDR_LGT;
  // How many bytes do we want to write
  packet_received.length = 6;
  // TWI address/commands to issue to the other chip (node)
  packet_received.addr[0] = VIRTUALMEM_ADDR_START >> 16;
  packet_received.addr[1] = VIRTUALMEM_ADDR_START >> 8;
  packet_received.addr[2] = VIRTUALMEM_ADDR_START;
  // Where to find the data to be written
  packet_received.buffer = data_received;

  // perform a read access
  status = twi_master_read(TWI, &packet_received);

  // check read result
  if (status == TWI_SUCCESS)
    print_dbg("\r\nRead Test:\tPASS");
  else
    print_dbg("\r\nRead test:\tFAIL");

  // check received data against sent data
  for (int i = 0 ; i < 6; i++)
  {
    if (data_received[i] != test_pattern[i])
      print_dbg("\r\nCheck Read:\tFAIL");
  }
*/

}


void twi_slave_rx( U8 u8_value )
{
  print_dbg("\r\ntwi_rx: ");
  print_dbg_hex(u8_value);
}

U8 twi_slave_tx( void )
{
   print_dbg("\r\ntwi_tx");
   return 0;
}


void twi_slave_stop( void )
{
   print_dbg("\r\ntwi_stop");
}




/*
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

*/
