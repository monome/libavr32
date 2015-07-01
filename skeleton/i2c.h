#ifndef _I2C_H_
#define _I2C_H_



extern void i2c_master_tx(void);
extern void i2c_master_tx(void);



extern void twi_slave_rx( U8 u8_value );
extern U8 twi_slave_tx( void );
extern void twi_slave_stop( void );

typedef void (*process_ii_t)(uint8_t, int);
extern volatile process_ii_t process_ii;



// #define I2C_BUFFER_SIZE		1024

// extern volatile u8 i2c_buffer[I2C_BUFFER_SIZE];


// // transmit data in slave mode
// extern u8 i2c_slave_tx(void);
// // handler for slave rx events
// extern void i2c_slave_rx(u8 value);

// // slave stop function
// extern void i2c_slave_stop(void);

// // master send
// extern void i2c_master_tx(u8* tx);
// // master receive
// extern void i2c_master_rx(u8* rx);




#endif // header guard
