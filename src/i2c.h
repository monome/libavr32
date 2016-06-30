#ifndef _I2C_H_
#define _I2C_H_

// PPP framing
#define STX 12
#define ETX 13
#define DLE 27

#define I2C_TX_BUF_SIZE 32
#define I2C_RX_BUF_SIZE 32

extern void i2c_master_tx(uint8_t, uint8_t *, uint8_t);
extern void i2c_master_rx(uint8_t addr, uint8_t idx);

extern void twi_slave_rx( U8 u8_value );
extern U8 twi_slave_tx( void );
extern void twi_slave_stop( void );

extern void ii_tx_queue(uint8_t);

typedef void (*process_ii_t)(uint8_t *d, uint8_t l);
extern volatile process_ii_t process_ii;


#endif // header guard