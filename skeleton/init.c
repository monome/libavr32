//ASF
#include <flashc.h>
#include <pll.h>
#include <sysclk.h>

#include "compiler.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "print_funcs.h"
#include "spi.h"
#include "tc.h"
#include "uhc.h"

#include "conf_board.h"
#include "conf_tc_irq.h"
#include "init.h"
#include "types.h"

//===================================
//==== external functions

extern void init_gpio(void) {
    gpio_enable_gpio_pin(B00);
    gpio_enable_gpio_pin(B01);
    gpio_enable_gpio_pin(B02);
    gpio_enable_gpio_pin(B03);
    gpio_enable_gpio_pin(B04);
    gpio_enable_gpio_pin(B05);
    gpio_enable_gpio_pin(B06);
    gpio_enable_gpio_pin(B07);
    gpio_enable_gpio_pin(B08);
    gpio_enable_gpio_pin(B09);
    gpio_enable_gpio_pin(B10);
    gpio_enable_gpio_pin(NMI);
}

// initialize application timer
extern void init_tc (void) {
  volatile avr32_tc_t *tc = APP_TC;

  // waveform options
  static const tc_waveform_opt_t waveform_opt = {
    .channel  = APP_TC_CHANNEL,  // channel
    .bswtrg   = TC_EVT_EFFECT_NOOP, // software trigger action on TIOB
    .beevt    = TC_EVT_EFFECT_NOOP, // external event action
    .bcpc     = TC_EVT_EFFECT_NOOP, // rc compare action
    .bcpb     = TC_EVT_EFFECT_NOOP, // rb compare
    .aswtrg   = TC_EVT_EFFECT_NOOP, // soft trig on TIOA
    .aeevt    = TC_EVT_EFFECT_NOOP, // etc
    .acpc     = TC_EVT_EFFECT_NOOP,
    .acpa     = TC_EVT_EFFECT_NOOP,
    // Waveform selection: Up mode with automatic trigger(reset) on RC compare.
    .wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
    .enetrg   = false,             // external event trig
    .eevt     = 0,                 // extern event select
    .eevtedg  = TC_SEL_NO_EDGE,    // extern event edge
    .cpcdis   = false,             // counter disable when rc compare
    .cpcstop  = false,            // counter stopped when rc compare
    .burst    = false,
    .clki     = false,
    // Internal source clock 5, connected to fPBA / 128.
    .tcclks   = TC_CLOCK_SOURCE_TC5
  };

  // Options for enabling TC interrupts
  static const tc_interrupt_t tc_interrupt = {
    .etrgs = 0,
    .ldrbs = 0,
    .ldras = 0,
    .cpcs  = 1, // Enable interrupt on RC compare alone
    .cpbs  = 0,
    .cpas  = 0,
    .lovrs = 0,
    .covfs = 0
  };
  // Initialize the timer/counter.
  tc_init_waveform(tc, &waveform_opt);

  // set timer compare trigger.
  // we want it to overflow and generate an interrupt every 1 ms
  // so (1 / fPBA / 128) * RC = 0.001
  // so RC = fPBA / 128 / 1000
  //  tc_write_rc(tc, APP_TC_CHANNEL, (FPBA_HZ / 128000));
  tc_write_rc(tc, APP_TC_CHANNEL, (FPBA_HZ / 128000));

  // configure the timer interrupt
  tc_configure_interrupts(tc, APP_TC_CHANNEL, &tc_interrupt);
  // Start the timer/counter.
  tc_start(tc, APP_TC_CHANNEL);
}


extern void init_spi (void) {
 
  sysclk_enable_pba_module(SYSCLK_SPI);

  static const gpio_map_t SPI_GPIO_MAP = {
    {SPI_SCK_PIN,  SPI_SCK_FUNCTION },
    {SPI_MISO_PIN, SPI_MISO_FUNCTION},
    {SPI_MOSI_PIN, SPI_MOSI_FUNCTION},
    {SPI_NPCS0_PIN,  SPI_NPCS0_FUNCTION },
    {SPI_NPCS1_PIN,  SPI_NPCS1_FUNCTION },
  };

  // Assign GPIO to SPI.
  gpio_enable_module(SPI_GPIO_MAP, sizeof(SPI_GPIO_MAP) / sizeof(SPI_GPIO_MAP[0]));


  spi_options_t spiOptions = {
    .reg = DAC_SPI,
    .baudrate = 2000000,
    .bits = 8,
    .trans_delay = 0,
    .spck_delay = 0,
    .stay_act = 1,
    .spi_mode = 1,
    .modfdis = 1
  };


  // Initialize as master.
  spi_initMaster(SPI, &spiOptions);
  // Set SPI selection mode: variable_ps, pcs_decode, delay.
  spi_selectionMode(SPI, 0, 0, 0);
  // Enable SPI module.
  spi_enable(SPI);

  // spi_setupChipReg( SPI, &spiOptions, FPBA_HZ );
  spi_setupChipReg(SPI, &spiOptions, sysclk_get_pba_hz() );


  // add ADC chip register
  spiOptions.reg          = ADC_SPI;
  spiOptions.baudrate     = 20000000;
  spiOptions.bits         = 16;
  spiOptions.spi_mode     = 2;
  spiOptions.spck_delay   = 0;
  spiOptions.trans_delay  = 5;
  spiOptions.stay_act     = 0;
  spiOptions.modfdis      = 0;

  spi_setupChipReg( SPI, &spiOptions, FPBA_HZ );

  // spi_enable(SPI);
 }

/*
// intialize two-wire interface
void init_twi(void) {
  // TWI/I2C GPIO map
  static const gpio_map_t TWI_GPIO_MAP = {
    { TWI_DATA_PIN, TWI_DATA_FUNCTION },
    { TWI_CLOCK_PIN, TWI_CLOCK_FUNCTION }
  };
  gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));
}
*/

// initialize USB host stack
void init_usb_host (void) {
  uhc_start();
}

