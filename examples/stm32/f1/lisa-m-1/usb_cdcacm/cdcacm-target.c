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
	rcc_clock_setup_in_hsi_out_48mhz();

	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_OTGFS);

	gpio_set(GPIOC, GPIO2);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
	gpio_set(GPIOC, GPIO5);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
}

const usbd_backend *cdcacm_target_usb_driver(void)
{
	return USBD_STM32_OTG_FS;
}

void cdcacm_target_usbd_after_init_and_before_first_poll(void)
{
	int i;

	for (i = 0; i < 0x800000; i++) {
		__asm__("nop");
	}

	gpio_clear(GPIOC, GPIO2);
}

extern void cdcacm_target_data_rx_cb_before_return(void)
{
	gpio_toggle(GPIOC, GPIO5);
}
