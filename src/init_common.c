#include "gpio.h"
#include "tc.h"
#include "twi.h"
#include "uhc.h"

#include "conf_tc_irq.h"
#include "init_common.h"
#include "i2c.h"

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

// initialize USB host stack
void init_usb_host (void) {
  uhc_start();
}

// initialize i2c
void init_i2c_leader(void) {
  twi_options_t opt;

  int status;

  static const gpio_map_t TWI_GPIO_MAP = {
    {AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},
    {AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}
  };

  gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));


  // options settings
  opt.pba_hz = FOSC0;
  opt.speed = TWI_SPEED;
  opt.chip = 0x50;

  // initialize TWI driver with options
  // status = twi_master_init(&AVR32_TWI, &opt);
  status = twi_master_init(TWI, &opt);
  // check init result
  // if (status == TWI_SUCCESS)
  //   print_dbg("\r\ni2c init");
  // else
  //   print_dbg("\r\ni2c init FAIL");
}

void init_i2c_follower(uint8_t addr) {
  static const gpio_map_t TWI_GPIO_MAP =
  {
    {AVR32_TWI_SDA_0_0_PIN, AVR32_TWI_SDA_0_0_FUNCTION},
    {AVR32_TWI_SCL_0_0_PIN, AVR32_TWI_SCL_0_0_FUNCTION}
  };
  twi_options_t opt;
  twi_slave_fct_t twi_slave_fct;
  int status;

  // TWI gpio pins configuration
  gpio_enable_module(TWI_GPIO_MAP, sizeof(TWI_GPIO_MAP) / sizeof(TWI_GPIO_MAP[0]));

  // options settings
  opt.pba_hz = FOSC0;
  opt.speed = TWI_SPEED;
  opt.chip = addr;

  // initialize TWI driver with options
  twi_slave_fct.rx = &twi_follower_rx;
  twi_slave_fct.tx = &twi_follower_tx;
  twi_slave_fct.stop = &twi_follower_stop;
  status = twi_slave_init(&AVR32_TWI, &opt, &twi_slave_fct );
/*  // check init result
  if (status == TWI_SUCCESS)
  {
    // display test result to user
    print_dbg("follower start:\tPASS\r\n");
  }
  else
  {
    // display test result to user
    print_dbg("follower start:\tFAIL\r\n");
  }*/
}
