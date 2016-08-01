/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2014 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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
#include <unicore-mx/stm32/crs.h>
#include <unicore-mx/stm32/syscfg.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/cm3/scb.h>
#include "cdcacm-target.h"

void cdcacm_target_init(void)
{
	/* start HSI48 */
	rcc_clock_setup_in_hsi48_out_48mhz();

	/* use usb SOF as correction source for HSI48 */
	crs_autotrim_usb_enable();

	/* use HSI48 for USB */
	rcc_set_usbclk_source(RCC_HSI48);

	/* usb HSI48 for system clock */
	rcc_set_sysclk_source(RCC_HSI48);

	rcc_periph_clock_enable(RCC_SYSCFG_COMP);
	SYSCFG_CFGR1 |= 1 << 4 /* PA11_PA12_RMP */;
}

const usbd_backend *cdcacm_target_usb_driver(void)
{
	return USBD_STM32_FSDEV_V2;
}
