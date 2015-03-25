/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2016 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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
#include <unicore-mx/efm32/cmu.h>
#include <unicore-mx/efm32/gpio.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/cm3/scb.h>
#include "cdcacm-target.h"

void cdcacm_target_init(void)
{
	cmu_clock_setup_in_hfxo_out_48mhz();
	cmu_periph_clock_enable(CMU_GPIO);

	gpio_mode_setup(GPIOE, GPIO_MODE_PUSH_PULL, GPIO2 | GPIO3);
}

void cdcacm_target_usbd_after_init_and_before_first_poll(void)
{
	unsigned i;

	for (i = 0; i < 0x800000; i++) {
		__asm__("nop");
	}

	gpio_clear(GPIOE, GPIO2);
}

void cdcacm_target_data_rx_cb_before_return(void)
{
	gpio_toggle(GPIOE, GPIO3);
}

const usbd_backend *cdcacm_target_usb_driver(void)
{
	return USBD_EFM32LG;
}
