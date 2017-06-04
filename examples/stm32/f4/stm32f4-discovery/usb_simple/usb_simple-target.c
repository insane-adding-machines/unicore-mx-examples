/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2015, 2016 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/usbd/usbd.h>
#include "usb_simple-target.h"

void usb_simple_target_init(void)
{
	rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_120MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_OTGFS);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO9 | GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);

	/* LED output */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO12 | GPIO13 | GPIO14 | GPIO15);
}

const usbd_backend *usb_simple_target_usb_driver(void)
{
	return USBD_STM32_OTG_FS;
}

void usb_simple_led_set_value(bool value)
{
	if (value) {
		gpio_set(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
	} else {
		gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);
	}
}
