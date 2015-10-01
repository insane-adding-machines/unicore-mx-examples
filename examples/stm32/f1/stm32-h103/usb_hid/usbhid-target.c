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
#include <unicore-mx/stm32/spi.h>
#include <unicore-mx/stm32/otg_fs.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/cm3/scb.h>
#include "usbhid-target.h"

void usbhid_target_init(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();

	rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set(GPIOC, GPIO11);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);
}

void usbhid_target_usbd_after_init_and_before_first_poll(void)
{
	unsigned i;

	for (i = 0; i < 0x80000; i++) {
		__asm__("nop");
	}

	gpio_clear(GPIOC, GPIO11);
}

const usbd_backend *usbhid_target_usb_driver(void)
{
	return USBD_STM32_FSDEV_V1;
}

void usbhid_detach_complete_before_scb_reset_core(void)
{
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, 0, GPIO15);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO10);
	gpio_set(GPIOA, GPIO10);
}
