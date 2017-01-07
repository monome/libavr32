#include "print_funcs.h"
#include "twi.h"
#include "events.h"
#include "i2c.h"

#define RX_BUFFER_COUNT 8

uint8_t rx_buffer[RX_BUFFER_COUNT][I2C_RX_BUF_SIZE];
static uint8_t rx_pos[RX_BUFFER_COUNT];
static uint8_t rx_buffer_index;

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

  // print_dbg("\r\nii_tx ");
  // for(int i =0;i< length;i++) {
  //   print_dbg_ulong(data[i]);
  //   print_dbg(" ");
  // }

  // perform a write access
  status = twi_master_write(TWI, &packet);
}


void i2c_master_rx(uint8_t addr, uint8_t *data, uint8_t l) {
  int status;

  packet.chip = addr;
  packet.addr_length = 0;
  // Where to find the data to be written
  packet.buffer = data;
  // How many bytes do we want to write
  packet.length = l;

  status = twi_master_read(TWI, &packet);

  //   print_dbg("\r\nii_rx ");
  // for(int i =0;i<l;i++) {
  //   print_dbg_ulong(data[i]);
  //   print_dbg(" ");
  // }
}



void twi_slave_rx( U8 u8_value )
{
  if (rx_pos[rx_buffer_index] < I2C_RX_BUF_SIZE) {
    rx_buffer[rx_buffer_index][rx_pos[rx_buffer_index]] = u8_value;
    rx_pos[rx_buffer_index]++;
  }
}

void twi_slave_stop( void )
{
  uint8_t index = rx_buffer_index;
  // rx_buffer_index must be incremented before process_ii is called
  if (++rx_buffer_index >= RX_BUFFER_COUNT) rx_buffer_index = 0;
  
  process_ii(rx_buffer[index], rx_pos[index]);
  rx_pos[index] = 0;
}




extern void ii_tx_queue(uint8_t data) {
  // print_dbg("\r\nii_tx_queue ");
  tx_buffer[tx_pos_write] = data;
  // print_dbg_ulong(data);

  tx_pos_write = (tx_pos_write + 1) & 7;
  if(tx_pos_write == tx_pos_read)
    print_dbg("\r\nii queue overrun");
}

uint8_t twi_slave_tx( void )
{
   // print_dbg("\r\nii_tx ");
   if(tx_pos_write == tx_pos_read)
    return 27;
  else {
    uint8_t d = tx_buffer[tx_pos_read];
    tx_pos_read = (tx_pos_read + 1) & 7;
    // print_dbg_ulong(d);
    return d;
  }
}