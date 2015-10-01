/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
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

#include <stdlib.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/cm3/scb.h>
#include "cdcacm-target.h"

void cdcacm_target_init(void)
{
	SCB_VTOR = (uint32_t) 0x08005000;

	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Setup GPIOC Pin 12 to pull up the D+ high, so autodect works
	 * with the bootloader.  The circuit is active low. */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN, GPIO12);
	gpio_clear(GPIOC, GPIO12);

	/* Setup GPIOA Pin 5 for the LED */
	gpio_set(GPIOA, GPIO5);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
}

void cdcacm_target_usbd_after_init_and_before_first_poll(void)
{
	unsigned i;

	for (i = 0; i < 0x800000; i++) {
		__asm__("nop");
	}

	gpio_clear(GPIOA, GPIO5);
}

const usbd_backend *cdcacm_target_usb_driver(void)
{
	return USBD_STM32_FSDEV_V1;
}
