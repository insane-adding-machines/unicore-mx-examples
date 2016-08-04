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
 * Generate clock for different part from 8Mhz clock
 * AHB = 168Mhz
 * APB1 = 42Mhz
 * APB2 = 84Mhz
 */
static void clock_setup(void)
{
	rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOD);

	rcc_periph_clock_enable(RCC_USART6);
	rcc_periph_clock_enable(RCC_OTGFS);
	rcc_periph_clock_enable(RCC_TIM6);
}

static void gpio_setup(void)
{
	/* Set GPIO12-15 (in GPIO port D) to 'output push-pull'. */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);

	/* Set */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
	gpio_clear(GPIOC, GPIO0);

	/* OTG_FS */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

	/* USART TX */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
	gpio_set_af(GPIOC, GPIO_AF8, GPIO6);
}

/**
 * Initalize the Output (TX) mode only (log)
 */
static void usart_init(void)
{
	usart_set_baudrate(USART6, 921600);
	usart_set_databits(USART6, 8);
	usart_set_flow_control(USART6, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART6, USART_MODE_TX);
	usart_set_parity(USART6, USART_PARITY_NONE);
	usart_set_stopbits(USART6, USART_STOPBITS_1);

	usart_enable(USART6);
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

	timer_reset(TIM6);
	timer_set_prescaler(TIM6, clock_div - 1);
	timer_set_period(TIM6, 0xFFFF);
	timer_enable_counter(TIM6);
}

/**
 * Get the counter value of Timer
 * @return Counter value
 */
uint16_t tim_get_counter(void)
{
	return timer_get_counter(TIM6);
}

/**
 * Output string @a arg
 */
void usart_puts(const char *arg)
{
	while (*arg != '\0') {
		usart_wait_send_ready(USART6);
		usart_send(USART6, *arg++);
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
	gpio_set(GPIOD,  GPIO13);
}

usbh_backend *usbh_kbd_backend(void)
{
	return USBH_STM32_OTG_FS;
}

void usbh_kbd_before_poll(void)
{
	/* Set busy led */
	gpio_set(GPIOD,  GPIO14);
}

void usbh_kbd_after_poll(void)
{
	unsigned i;

	/* Clear busy led */
	gpio_clear(GPIOD,  GPIO14);

	/* dummy delay, approx 1ms interval */
	for (i = 0; i < 14903; i++) {}
}
