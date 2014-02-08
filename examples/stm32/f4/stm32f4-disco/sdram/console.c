/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2013 Chuck McManis <cmcmanis@mcmanis.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * USART example (alternate console)
 *
 * This version then adds in interrupts, which is really handy for
 * the receive function as it is impossible to predict when someone
 * might type a character, further you can create a "character based
 * reset" capability if you choose to.
 */

#include <stdint.h>
#include <setjmp.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/cortex.h>
#include "console.h"


/*
 * Some definitions of our console "functions" attached to the 
 * USART.
 *
 * These define sort of the minimum "library" of functions which
 * we can use on a serial port.
 */


#define CONSOLE_UART	USART2


/* This is a ring buffer to holding characters as they are typed
 * it maintains both the place to put the next character received
 * from the UART, and the place where the last character was
 * read by the program. See the README file for a discussion of
 * the failure semantics.
 */
#define RECV_BUF_SIZE	128		// Arbitrary buffer size
char recv_buf[RECV_BUF_SIZE];
volatile int recv_ndx_nxt;		// Next place to store
volatile int recv_ndx_cur;		// Next place to read

/* For interrupt handling we add a new function which is called
 * when recieve interrupts happen. The name (usart2_isr) is created
 * by the irq.json file in libopencm3 calling this interrupt for
 * USART2 'usart2', adding the suffix '_isr', and then weakly binding
 * it to the 'do nothing' interrupt function in vec.c.
 *
 * By defining it in this file the linker will override that weak
 * binding and instead bind it here, but you have to get the name
 * right or it won't work. And you'll wonder where your interrupts
 * are going.
 */
void usart2_isr(void) {
	uint32_t	reg;
	int			i;

	do {
		reg = USART_SR(CONSOLE_UART);
		if (reg & USART_SR_RXNE) {
			recv_buf[recv_ndx_nxt] = USART_DR(CONSOLE_UART);
#ifdef RESET_ON_CTRLC
			/* Check for "reset" */
			if (recv_buf[recv_ndx_nxt] == '\003') {
				/* reset the system */
				volatile uint32_t *ret = (&reg) + 7;	// Return address on stack
				
				*ret = (uint32_t) &reset_handler;		// force system reset
				return;									// go to new address
			}
#endif
			/* Check for "overrun" */
			i = (recv_ndx_nxt + 1) % RECV_BUF_SIZE;
			if (i != recv_ndx_cur) {
				recv_ndx_nxt = i;
			}
		}
	} while ((reg & USART_SR_RXNE) != 0); // can read back-to-back interrupts
}

/*
 * console_putc(char c)
 *
 * Send the character 'c' to the USART, wait for the USART
 * transmit buffer to be empty first.
 */
void console_putc(char c) {
	uint32_t	reg;
	do {
		reg = USART_SR(CONSOLE_UART);
	} while ((reg & USART_SR_TXE) == 0);
	USART_DR(CONSOLE_UART) = (uint16_t) c & 0xff;
}

/*
 * char = console_getc(int wait)
 *
 * Check the console for a character. If the wait flag is
 * non-zero. Continue checking until a character is received
 * otherwise return 0 if called and no character was available.
 *
 * The implementation is a bit different however, now it looks
 * in the ring buffer to see if a character has arrived.
 */
char console_getc(int wait) {
	char		c = 0;

	while ((wait != 0) && (recv_ndx_cur == recv_ndx_nxt)) ;
	if (recv_ndx_cur != recv_ndx_nxt) {
		c = recv_buf[recv_ndx_cur];
		recv_ndx_cur = (recv_ndx_cur + 1) % RECV_BUF_SIZE;
	}
	return c;
}

/*
 * void console_puts(char *s)
 *
 * Send a string to the console, one character at a time, return
 * after the last character, as indicated by a NUL character, is
 * reached.
 */
void console_puts(char *s) {
	while (*s != '\000') {
		console_putc(*s);
		/* Add in a carraige return, after sending line feed */
		if (*s == '\n') {
			console_putc('\r');
		}
		s++;
	}
}

/*
 * int console_gets(char *s, int len)
 *
 * Wait for a string to be entered on the console, limited
 * support for editing characters (back space and delete)
 * end when a <CR> character is received.
 */
int console_gets(char *s, int len) {
	char *t = s;
	char c;

	*t = '\000';
	/* read until a <CR> is received */
	while ((c = console_getc(1)) != '\r') {
		if ((c == '\010') || (c == '\127')) {
			if (t > s) {
				/* send ^H ^H to erase previous character */
				console_puts("\010 \010");
				t--;
			}
		} else {
			*t = c;
			console_putc(c);
			if ((t - s) < len) {
				t++;
			}
		}
		/* update end of string with NUL */
		*t = '\000';
	}
	return (t - s);
}

/*
 * Set up the GPIO subsystem with an "Alternate Function"
 * on some of the pins, in this case connected to a
 * USART.
 */
void console_setup(void) {

	/* MUST enable the GPIO clock in ADDITION to the USART clock */
	rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPDEN);

	/* This example uses PD5 and PD6 for Tx and Rx respectively
	 * but other pins are available for this role on USART2 (our chosen
 	 * USART) as well, such as PA2 and PA3. You can also split them
	 * so PA2 for Tx, PD6 for Rx but you would have to enable both
	 * the GPIOA and GPIOD clocks in that case
	 */
	gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO6);

	/* Actual Alternate function number (in this case 7) is part
	 * depenedent, check the data sheet for the right number to
	 * use.
	 */
	gpio_set_af(GPIOD, GPIO_AF7, GPIO5 | GPIO6);


	/* This then enables the clock to the USART2 peripheral which is
	 * attached inside the chip to the APB2 bus. Different peripherals
	 * attach to different buses, and even some UARTS are attached to
	 * APB1 and some to APB2, again the data sheet is useful here.
	 */
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USART2EN);

	/* Set up USART/UART parameters using the libopencm3 helper functions */
	usart_set_baudrate(CONSOLE_UART, 115200);
	usart_set_databits(CONSOLE_UART, 8);
	usart_set_stopbits(CONSOLE_UART, USART_STOPBITS_1);
	usart_set_mode(CONSOLE_UART, USART_MODE_TX_RX);
	usart_set_parity(CONSOLE_UART, USART_PARITY_NONE);
	usart_set_flow_control(CONSOLE_UART, USART_FLOWCONTROL_NONE);
	usart_enable(CONSOLE_UART);

	/* Enable interrupts from the USART */
	nvic_enable_irq(NVIC_USART2_IRQ);

	/* Specifically enable recieve interrupts */
	usart_enable_rx_interrupt(CONSOLE_UART);
}