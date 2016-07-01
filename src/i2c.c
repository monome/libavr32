#include "print_funcs.h"
#include "twi.h"
#include "events.h"
#include "i2c.h"

uint8_t rx_buffer[I2C_RX_BUF_SIZE];
static uint8_t rx_pos;

uint8_t tx_buffer[I2C_TX_BUF_SIZE];
static uint8_t tx_pos_read;
static uint8_t tx_pos_write;

static twi_package_t packet;
// static twi_package_t packet_received;

volatile process_ii_t process_ii;


void i2c_master_tx(uint8_t addr, uint8_t *data, uint8_t length) {
  int status;

  packet.chip = addr;
  packet.addr_length = 0;
  // Where to find the data to be written
  packet.buffer = data;
  // How many bytes do we want to write
  packet.length = length;

  // perform a write access
  status = twi_master_write(TWI, &packet);
}


void i2c_master_rx(uint8_t addr, uint8_t *data, uint8_t l) {
  int status;

  packet.chip = addr;
  // TWI address/commands to issue to the other chip (node)
  packet.addr[0] = 77;
  // packet.addr[0] = idx;
  // packet.addr[1] = VIRTUALMEM_ADDR_START >> 8;
  // packet.addr[2] = VIRTUALMEM_ADDR_START;
  // Length of the TWI data address segment (1-3 bytes)
  // packet.addr_length = EEPROM_ADDR_LGT;
  packet.addr_length = 1;
  // Where to find the data to be written
  packet.buffer = data;
  // How many bytes do we want to write
  packet.length = l;

  status = twi_master_read(TWI, &packet);

  // print_dbg("\r\nii receive: ");
  // print_dbg_ulong(d[0]);
}



void twi_slave_rx( U8 u8_value )
{
  rx_buffer[rx_pos] = u8_value;
  rx_pos++;
}

void twi_slave_stop( void )
{
  // print_dbg("\r\nrx ");

  // for(int n=0;n<rx_pos;n++) {
  //   print_dbg("\r\n");
  //   print_dbg_hex(rx_buffer[n]);
  // }

  // int i = (rx_buffer[1] << 8) + rx_buffer[2];

  // print_dbg("\r\ntwi: ");
  // print_dbg_ulong(rx_buffer[0]);
  // print_dbg(" ");
  // print_dbg_ulong(i);
  process_ii(&rx_buffer[0], rx_pos);
  rx_pos = 0;
}




extern void ii_tx_queue(uint8_t data) {
  print_dbg("\r\nii_tx_queue ");
  tx_buffer[tx_pos_write] = data;
  print_dbg_ulong(data);

  tx_pos_write = (tx_pos_write + 1) & 7;
  if(tx_pos_write == tx_pos_read)
    print_dbg("\r\nii queue overrun");
}

uint8_t twi_slave_tx( void )
{
   print_dbg("\r\nii_tx ");
   if(tx_pos_write == tx_pos_read)
    return 27;
  else {
    uint8_t d = tx_buffer[tx_pos_read];
    tx_pos_read = (tx_pos_read + 1) & 7;
    print_dbg_ulong(d);
    return d;
  }
}