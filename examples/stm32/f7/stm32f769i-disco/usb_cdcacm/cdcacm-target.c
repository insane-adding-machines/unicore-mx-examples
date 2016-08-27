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
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/stm32/usart.h>
#include <unicore-mx/stm32/otg_hs.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/cm3/scb.h>
#include "cdcacm-target.h"

/* Speed selection only, External PHY only */
/* #define SHOW_HIGH_SPEED_DEMO */

static void ulpi_pins(uint32_t gpioport, uint16_t gpiopins)
{
	gpio_mode_setup(gpioport, GPIO_MODE_AF, GPIO_PUPD_NONE, gpiopins);
	gpio_set_output_options(gpioport, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, gpiopins);
	gpio_set_af(gpioport, GPIO_AF10, gpiopins);
}

void cdcacm_target_init(void)
{
	/* Base board frequency, set to 216MHz */
	rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOH);
	rcc_periph_clock_enable(RCC_GPIOI);
	rcc_periph_clock_enable(RCC_OTGHSULPI);
	rcc_periph_clock_enable(RCC_OTGHS);

	/*
	 * ULPI   GPIO
	 *  D0  -> PA3
	 *  D1  -> PB0
	 *  D2  -> PB1
	 *  D3  -> PB10
	 *  D4  -> PB11
	 *  D5  -> PB12
	 *  D6  -> PB13
	 *  D7  -> PB5
	 *  CK  -> PA5
	 *  STP -> PC0
	 *  NXT -> PH4
	 *  DIR -> PI11
	 */
	ulpi_pins(GPIOA, GPIO3 | GPIO5);
	ulpi_pins(GPIOB, GPIO0 | GPIO1 | GPIO5  | GPIO10 | GPIO11 | GPIO12 | GPIO13);
	ulpi_pins(GPIOC, GPIO0);
	ulpi_pins(GPIOH, GPIO4);
	ulpi_pins(GPIOI, GPIO11);
}

static const usbd_backend_config _config = {
	.ep_count = 9,
	.priv_mem = 4096,
#if defined(SHOW_HIGH_SPEED_DEMO)
	.speed = USBD_SPEED_HIGH,
#else
	.speed = USBD_SPEED_FULL,
#endif
	.feature = USBD_PHY_EXT
};

const usbd_backend_config *cdcacm_target_usb_config(void)
{
	return &_config;
}

const usbd_backend *cdcacm_target_usb_driver(void)
{
	return USBD_STM32_OTG_HS;
}
