//ASF
#include <sysclk.h>

#include "gpio.h"
#include "spi.h"
#include "intc.h"
#include "print_funcs.h"
#include "tc.h"
#include "usart.h"

#include "conf_board.h"
#include "conf_tc_irq.h"
#include "events.h"
#include "timers.h"
#include "types.h"
#include "init_teletype.h"


//#define UI_IRQ_PRIORITY AVR32_INTC_INT2


//------------------------
//----- variables
// timer tick counter
static volatile u64 tcTicks = 0;
static volatile u8 tcOverflow = 0;
static const u64 tcMax = (U64)0x7fffffff;
static const u64 tcMaxInv = (u64)0x10000000;

//----------------------
//---- static functions
// interrupt handlers

// irq for app timer
__attribute__((__interrupt__))
static void irq_tc(void);


// irq for PA00-PA07
__attribute__((__interrupt__))
static void irq_port0_line0(void);

// irq for PA08-PA15
__attribute__((__interrupt__))
static void irq_port0_line1(void);



// irq for uart
// __attribute__((__interrupt__))
// static void irq_usart(void);


//---------------------------------
//----- static function definitions


// timer irq
__attribute__((__interrupt__))
static void irq_tc(void) {
  tcTicks++;
  // overflow control
  if(tcTicks > tcMax) {
    tcTicks = 0;
    tcOverflow = 1;
  } else {
    tcOverflow = 0;
  }
  process_timers();
  // clear interrupt flag by reading timer SR
  tc_read_sr(APP_TC, APP_TC_CHANNEL);
}


// interrupt handler for PA00-PA07
__attribute__((__interrupt__))
static void irq_port0_line0(void) {
  for(int i=0;i<8;i++) {
    if(gpio_get_pin_interrupt_flag(i)) {
      // print_dbg("\r\n # A00");
      event_t e = { .type = kEventTrigger, .data = i };
      event_post(&e);
      gpio_clear_pin_interrupt_flag(i);
    }
  }
}

// interrupt handler for PA08-PA15
__attribute__((__interrupt__))
static void irq_port0_line1(void) {
    if(gpio_get_pin_interrupt_flag(NMI)) {
      // print_dbg("\r\n ### NMI ### ");
      event_t e = { .type = kEventFront, .data = gpio_get_pin_value(NMI) };
      event_post(&e);
      gpio_clear_pin_interrupt_flag(NMI);
    }
}

// interrupt handler for uart
// __attribute__((__interrupt__))
// static void irq_usart(void) {
// }

//===================================
//==== external functions

// register interrupts
void register_interrupts(void) {
  // enable interrupts on GPIO inputs
  gpio_enable_pin_interrupt( NMI, GPIO_PIN_CHANGE);
  // gpio_enable_pin_interrupt( B08, GPIO_PIN_CHANGE);
  // gpio_enable_pin_interrupt( B09,  GPIO_PIN_CHANGE);

  gpio_enable_pin_interrupt( A00, GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt( A01, GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt( A02, GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt( A03, GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt( A04, GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt( A05, GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt( A06, GPIO_RISING_EDGE);
  gpio_enable_pin_interrupt( A07,	GPIO_RISING_EDGE);

  // PA00-A07
  INTC_register_interrupt( &irq_port0_line0, AVR32_GPIO_IRQ_0, UI_IRQ_PRIORITY);

  // PA08 - PA15
  INTC_register_interrupt( &irq_port0_line1, AVR32_GPIO_IRQ_0 + (AVR32_PIN_PA08 / 8), UI_IRQ_PRIORITY);

  // PB08 - PB15
  // INTC_register_interrupt( &irq_port1_line1, AVR32_GPIO_IRQ_0 + (AVR32_PIN_PB08 / 8), UI_IRQ_PRIORITY);

  // register TC interrupt
  INTC_register_interrupt(&irq_tc, APP_TC_IRQ, APP_TC_IRQ_PRIORITY);

  // register uart interrupt
  // INTC_register_interrupt(&irq_usart, AVR32_USART0_IRQ, UI_IRQ_PRIORITY);
}

extern void init_gpio(void) {
	gpio_enable_gpio_pin(A00);
	gpio_enable_gpio_pin(A01);
	gpio_enable_gpio_pin(A02);
	gpio_enable_gpio_pin(A03);
	gpio_enable_gpio_pin(A04);
	gpio_enable_gpio_pin(A05);
	gpio_enable_gpio_pin(A06);
	gpio_enable_gpio_pin(A07);

	gpio_enable_gpio_pin(B08);
	gpio_enable_gpio_pin(B09);
	gpio_enable_gpio_pin(B10);
	gpio_enable_gpio_pin(B11);

  // turn on pull-ups for SDA/SCL
  // gpio_enable_pin_pull_up(A09);
  // gpio_enable_pin_pull_up(A10);

	gpio_enable_gpio_pin(NMI);

	gpio_configure_pin(B08, GPIO_DIR_OUTPUT);
	gpio_configure_pin(B09, GPIO_DIR_OUTPUT);
	gpio_configure_pin(B10, GPIO_DIR_OUTPUT);
	gpio_configure_pin(B11, GPIO_DIR_OUTPUT);
}


extern void init_spi (void) {

	sysclk_enable_pba_module(SYSCLK_SPI);

	static const gpio_map_t SPI_GPIO_MAP = {
		{SPI_SCK_PIN,  SPI_SCK_FUNCTION },
		{SPI_MISO_PIN, SPI_MISO_FUNCTION},
		{SPI_MOSI_PIN, SPI_MOSI_FUNCTION},
		{SPI_NPCS0_PIN,  SPI_NPCS0_FUNCTION },
		{SPI_NPCS1_PIN,  SPI_NPCS1_FUNCTION },
		{SPI_NPCS2_PIN,  SPI_NPCS2_FUNCTION },
	};

  // Assign GPIO to SPI.
	gpio_enable_module(SPI_GPIO_MAP, sizeof(SPI_GPIO_MAP) / sizeof(SPI_GPIO_MAP[0]));


	spi_options_t spiOptions = {
		.reg = DAC_SPI_NPCS,
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
	spiOptions.reg          = ADC_SPI_NPCS;
	spiOptions.baudrate     = 20000000;
	spiOptions.bits         = 16;
	spiOptions.spi_mode     = 2;
	spiOptions.spck_delay   = 0;
	spiOptions.trans_delay  = 5;
	spiOptions.stay_act     = 0;
	spiOptions.modfdis      = 0;

	spi_setupChipReg( SPI, &spiOptions, FPBA_HZ );


  // add OLED chip register
	spiOptions.reg          = OLED_SPI_NPCS;
	spiOptions.baudrate     = 40000000;
	spiOptions.bits         = 8;
	spiOptions.spi_mode     = 3;
	spiOptions.spck_delay   = 0;
	spiOptions.trans_delay  = 0;
	spiOptions.stay_act     = 1;
	spiOptions.modfdis      = 1;

	spi_setupChipReg( SPI, &spiOptions, FPBA_HZ );
}

extern u64 get_ticks(void) {
	return tcTicks;
}