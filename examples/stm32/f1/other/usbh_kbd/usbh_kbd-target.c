/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2016 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
 * Copyright (C) 2015 Amir Hammad <amir.hammad@hotmail.com>
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

#include "usbh_kbd-target.h"

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/usart.h>
#include <unicore-mx/stm32/timer.h>
#include <unicore-mx/stm32/otg_fs.h>

/**
 * Generate clock for different part from 16Mhz clock
 * AHB = 72Mhz
 * APB1 = 36Mhz
 * APB2 = 72Mhz
 */
static void clock_setup(void)
{
	rcc_clock_setup_in_hse_16mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_OTGFS);
	rcc_periph_clock_enable(RCC_UART4);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_TIM4);
}

static void gpio_setup(void)
{
	/* OTG_FS */
	gpio_set_mode(GPIOC, GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_ANALOG, GPIO11 | GPIO12);

	/* USART TX */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO10);
}

/**
 * Initalize the Output (TX) mode only (log)
 */
static void usart_init(void)
{
	usart_set_baudrate(UART4, 921600);
	usart_set_databits(UART4, 8);
	usart_set_flow_control(UART4, USART_FLOWCONTROL_NONE);
	usart_set_mode(UART4, USART_MODE_TX);
	usart_set_parity(UART4, USART_PARITY_NONE);
	usart_set_stopbits(UART4, USART_STOPBITS_1);

	usart_enable(UART4);
}

/**
 * Setup the timer in 10Khz mode
 * This timer will be used to keep track of last poll time difference
 */
static void tim_setup(void)
{
	uint32_t tick_freq = 10000;

	uint32_t timer_clock = rcc_apb1_frequency;
	if (rcc_ahb_frequency != rcc_apb1_frequency) {
		timer_clock *= 2;
	}

	uint32_t clock_div = timer_clock / tick_freq;

	timer_reset(TIM4);
	timer_set_prescaler(TIM4, clock_div - 1);
	timer_set_period(TIM4, 0xFFFF);
	timer_enable_counter(TIM4);
}

/**
 * Get the counter value of Timer
 * @return Counter value
 */
uint16_t tim_get_counter(void)
{
	return timer_get_counter(TIM4);
}

/**
 * Output string @a arg
 */
void usart_puts(const char *arg)
{
	while (*arg != '\0') {
		usart_wait_send_ready(UART4);
		usart_send(UART4, *arg++);
	}
}

void usart_vprintf(const char *fmt, va_list va)
{
	char db[128];
	if (vsnprintf(db, sizeof(db), fmt, va) > 0) {
		usart_puts(db);
	}
}

void usart_printf(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	usart_vprintf(fmt, va);
	va_end(va);
}

void usbh_kbd_init(void)
{
	clock_setup();
	gpio_setup();

	tim_setup();

	usart_init();
}

usbh_backend *usbh_kbd_backend(void)
{
	return USBH_STM32_OTG_FS;
}

void usbh_kbd_before_poll(void)
{
}

void usbh_kbd_after_poll(void)
{
	/* dummy delay, approx 1ms interval */
	unsigned i;
	for (i = 0; i < 14903; i++) {}
}
