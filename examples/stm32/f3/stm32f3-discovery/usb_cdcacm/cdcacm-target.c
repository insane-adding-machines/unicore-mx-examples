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

static void usb_setup(void)
{
	/* Enable clocks for GPIO port A (for GPIO_USART2_TX) and USART2. */
	rcc_usb_prescale_1();
	rcc_periph_clock_enable(RCC_USB);
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Setup GPIO pin GPIO_USART2_TX/GPIO9 on GPIO port A for transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF14, GPIO11| GPIO12);
}

void cdcacm_target_init(void)
{
	rcc_clock_setup_hsi(&rcc_hsi_8mhz[RCC_CLOCK_48MHZ]);

	usb_setup();
}

const usbd_backend *cdcacm_target_usb_driver(void)
{
	return USBD_STM32_FSDEV;
}

void cdcacm_target_usbd_after_init_and_before_first_poll(void)
{
	int i;

	for (i = 0; i < 0x800000; i++) {
		__asm__("nop");
	}
}
