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

#include <unicore-mx/efm32/cmu.h>
#include <unicore-mx/efm32/gpio.h>
#include <unicore-mx/usbd/usbd.h>
#include "usb_simple-target.h"

void usb_simple_target_init(void)
{
	cmu_clock_setup_in_hfxo_out_48mhz();

	cmu_periph_clock_enable(CMU_GPIO);

	/* LED output */
	gpio_mode_setup(GPIOE, GPIO_MODE_PUSH_PULL, GPIO2);

	/* USB Pins */
	gpio_mode_setup(GPIOF, GPIO_MODE_DISABLE, GPIO10 | GPIO11);
}

const usbd_backend *usb_simple_target_usb_driver(void)
{
	return USBD_EFM32LG;
}

void usb_simple_led_set_value(bool value)
{
	if (value) {
		gpio_set(GPIOE, GPIO2);
	} else {
		gpio_clear(GPIOE, GPIO2);
	}
}
