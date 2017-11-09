/**
 * \file
 *
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 */

#ifndef _TRILOGY_CONF_BOARD_H_
#define _TRILOGY_CONF_BOARD_H_

// #warning including conf_board.h (trilogy)

#include "compiler.h"

#define FOSC32          32768
#define OSC32_STARTUP   AVR32_PM_OSCCTRL32_STARTUP_8192_RCOSC

#define FOSC0           12000000
#define OSC0_STARTUP    AVR32_PM_OSCCTRL0_STARTUP_2048_RCOSC


/* These are documented in services/basic/clock/uc3b0_b1/osc.h */
#define BOARD_OSC0_HZ           12000000
#define BOARD_OSC0_STARTUP_US   17000
#define BOARD_OSC0_IS_XTAL      true
#define BOARD_OSC32_HZ          32768
#define BOARD_OSC32_STARTUP_US  71000
#define BOARD_OSC32_IS_XTAL     true

// #define CONFIG_SYSCLK_SOURCE    SYSCLK_SRC_OSC0
#define CONFIG_SYSCLK_SOURCE    SYSCLK_SRC_PLL0
#define CONFIG_SYSCLK_CPU_DIV   0
#define CONFIG_SYSCLK_PBA_DIV   0


#define FMCK_HZ   		60000000   // master */
#define FCPU_HZ   		FMCK_HZ    // cpu */
#define FHSB_HZ   		FCPU_HZ    // highspeed bus */
#define FPBA_HZ   		FMCK_HZ    // peripheral bus A */
#define FPBB_HZ   		FMCK_HZ    // peripheral bus B */


#define B00   AVR32_PIN_PB00
#define B01   AVR32_PIN_PB01
#define B02   AVR32_PIN_PB02
#define B03   AVR32_PIN_PB03
#define B04   AVR32_PIN_PB04
#define B05   AVR32_PIN_PB05
#define B06   AVR32_PIN_PB06
#define B07   AVR32_PIN_PB07
#define B08   AVR32_PIN_PB08
#define B09   AVR32_PIN_PB09
#define B10   AVR32_PIN_PB10
#define A09   AVR32_PIN_PA09
#define A10   AVR32_PIN_PA10
#define NMI   AVR32_PIN_PA13


#define USART                       (&AVR32_USART1)
#define USART_RXD_PIN               AVR32_USART1_RXD_0_0_PIN
#define USART_RXD_FUNCTION          AVR32_USART1_RXD_0_0_FUNCTION
#define USART_TXD_PIN               AVR32_USART1_TXD_0_0_PIN
#define USART_TXD_FUNCTION          AVR32_USART1_TXD_0_0_FUNCTION
#define USART_IRQ                   AVR32_USART1_IRQ
#define USART_IRQ_GROUP             AVR32_USART1_IRQ_GROUP
#define USART_SYSCLK                SYSCLK_USART1
#define USART_BAUDRATE 				115200


/// compatibility with ASF print funcs
#define DBG_USART              USART
#define DBG_USART_RX_PIN       USART_RXD_PIN
#define DBG_USART_RX_FUNCTION  USART_RXD_FUNCTION
#define DBG_USART_TX_PIN       USART_TXD_PIN
#define DBG_USART_TX_FUNCTION  USART_TXD_FUNCTION
#define DBG_USART_BAUDRATE     USART_BAUDRATE


//! Multiplexed pin used for USB_ID: AVR32_USBB_USB_ID_x_x.
//! To be selected according to the AVR32_USBB_USB_ID_x_x_PIN and
//! AVR32_USBB_USB_ID_x_x_FUNCTION definitions from <avr32/uc3bxxxx.h>.
// USB_ID on PA26
#define USB_ID                      AVR32_USBB_USB_ID_0_0
//! Multiplexed pin used for USB_VBOF: AVR32_USBB_USB_VBOF_x_x.
//! To be selected according to the AVR32_USBB_USB_VBOF_x_x_PIN and
//! AVR32_USBB_USB_VBOF_x_x_FUNCTION definitions from <avr32/uc3bxxxx.h>.
// USB_VBOF on PA27
#define USB_VBOF                    AVR32_USBB_USB_VBOF_0_0
#define USB_VBOF_ACTIVE_LEVEL       LOW
// USB_OC on PA25
#define USB_OVERCURRENT_DETECT_PIN  AVR32_PIN_PA25


#define SPARE_TWI                   (&AVR32_TWI)
#define SPARE_TWI_SCL_PIN           AVR32_TWI_SCL_0_0_PIN
#define SPARE_TWI_SCL_FUNCTION      AVR32_TWI_SCL_0_0_FUNCTION
#define SPARE_TWI_SDA_PIN           AVR32_TWI_SDA_0_0_PIN
#define SPARE_TWI_SDA_FUNCTION      AVR32_TWI_SDA_0_0_FUNCTION

// SCK A15
// MISO A28
// MOSI A29
// CS0 A16
// CS1 A17
#define SPI                   (&AVR32_SPI)
#define SPI_SCK_PIN           AVR32_SPI_SCK_0_0_PIN
#define SPI_SCK_FUNCTION      AVR32_SPI_SCK_0_0_FUNCTION
#define SPI_MISO_PIN          AVR32_SPI_MISO_0_2_PIN
#define SPI_MISO_FUNCTION     AVR32_SPI_MISO_0_2_FUNCTION
#define SPI_MOSI_PIN          AVR32_SPI_MOSI_0_2_PIN
#define SPI_MOSI_FUNCTION     AVR32_SPI_MOSI_0_2_FUNCTION
#define SPI_NPCS0_PIN         AVR32_SPI_NPCS_0_0_PIN
#define SPI_NPCS0_FUNCTION    AVR32_SPI_NPCS_0_0_FUNCTION
#define SPI_NPCS1_PIN         AVR32_SPI_NPCS_1_0_PIN
#define SPI_NPCS1_FUNCTION    AVR32_SPI_NPCS_1_0_FUNCTION

#define DAC_SPI_NPCS 0
#define ADC_SPI_NPCS 1

// compatibility with aleph
#define ADC_SPI SPI
#define DAC_SPI SPI

  //TWI
#define TWI 					(&AVR32_TWI)
#define TWI_DATA_PIN			AVR32_TWI_SDA_0_0_PIN
#define TWI_DATA_FUNCTION   	AVR32_TWI_SDA_0_0_FUNCTION
#define TWI_CLOCK_PIN           AVR32_TWI_SCL_0_0_PIN
#define TWI_CLOCK_FUNCTION  	AVR32_TWI_SCL_0_0_FUNCTION
#define TWI_SPEED				100000

#define MOD_TRILOGY

#endif // CONF_BOARD_H
