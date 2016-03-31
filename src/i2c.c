#include "print_funcs.h"
#include "twi.h"
#include "events.h"
#include "i2c.h"

static volatile uint8_t rx_buffer[8];
static uint8_t rx_pos;

static volatile uint8_t tx_buffer[8];
static uint8_t tx_pos_read;
static uint8_t tx_pos_write;

static twi_package_t packet;
// static twi_package_t packet_received;

volatile process_ii_t process_ii;


extern void ii_tx_queue(uint8_t data) {
  tx_buffer[tx_pos_write] = data;

  tx_pos_write = (tx_pos_write + 1) & 7;
  if(tx_pos_write == tx_pos_read)
    print_dbg("\r\nii queue overrun");
}


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


void i2c_master_rx(uint8_t addr, uint8_t idx) {
  int status;
  uint8_t d[3];

  packet.chip = addr;
  // TWI address/commands to issue to the other chip (node)
  packet.addr[0] = idx;
  // packet.addr[1] = VIRTUALMEM_ADDR_START >> 8;
  // packet.addr[2] = VIRTUALMEM_ADDR_START;
  // Length of the TWI data address segment (1-3 bytes)
  // packet.addr_length = EEPROM_ADDR_LGT;
  packet.addr_length = 1;
  // Where to find the data to be written
  packet.buffer = &d;
  // How many bytes do we want to write
  packet.length = 2;

  status = twi_master_read(TWI, &packet);

  print_dbg("\r\nii receive: ");
  print_dbg_ulong(d[0]);
}


void twi_slave_rx( U8 u8_value )
{
  rx_buffer[rx_pos] = u8_value;
  rx_pos++;
}

U8 twi_slave_tx( void )
{
   print_dbg("\r\ntwi_tx");
   if(tx_pos_write == tx_pos_read)
    return 27;
  else {
    uint8_t d = tx_buffer[tx_pos_read];
    tx_pos_read = (tx_pos_read + 1) & 7;
    return d;
  }
}


void twi_slave_stop( void )
{
  // print_dbg("\r\nrx ");

  // for(int n=0;n<rx_pos;n++) {
  //   print_dbg("\r\n");
  //   print_dbg_hex(rx_buffer[n]);
  // }

  int i = (rx_buffer[1] << 8) + rx_buffer[2];
  rx_pos = 0;
  // print_dbg("\r\ntwi: ");
  // print_dbg_ulong(rx_buffer[0]);
  // print_dbg(" ");
  // print_dbg_ulong(i);
  process_ii(rx_buffer[0],i);
}
