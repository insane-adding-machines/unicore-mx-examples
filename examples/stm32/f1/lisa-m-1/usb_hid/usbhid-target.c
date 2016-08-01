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
#include "adxl345.h"

static uint8_t spi_readwrite(uint32_t spi, uint8_t data)
{
	while (SPI_SR(spi) & SPI_SR_BSY);
	SPI_DR(spi) = data;
	while (!(SPI_SR(spi) & SPI_SR_RXNE));
	return SPI_DR(spi);
}

static uint8_t accel_read(uint8_t addr)
{
	uint8_t ret;
	gpio_clear(GPIOB, GPIO12);
	spi_readwrite(SPI2, addr | 0x80);
	ret = spi_readwrite(SPI2, 0);
	gpio_set(GPIOB, GPIO12);
	return ret;
}

static void accel_write(uint8_t addr, uint8_t data)
{
	gpio_clear(GPIOB, GPIO12);
	spi_readwrite(SPI2, addr);
	spi_readwrite(SPI2, data);
	gpio_set(GPIOB, GPIO12);
}

static void accel_get(int16_t *x, int16_t *y, int16_t *z)
{
	if (x != NULL) {
		*x = accel_read(ADXL345_DATAX0) |
			(accel_read(ADXL345_DATAX1) << 8);
	}

	if (y != NULL) {
		*y = accel_read(ADXL345_DATAY0) |
			(accel_read(ADXL345_DATAY1) << 8);
	}

	if (z != NULL) {
		*z = accel_read(ADXL345_DATAZ0) |
			(accel_read(ADXL345_DATAZ1) << 8);
	}
}

void usbhid_target_init(void)
{
	rcc_clock_setup_in_hse_12mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_SPI2);
	rcc_periph_clock_enable(RCC_OTGFS);

	/* Configure SPI2: PB13(SCK), PB14(MISO), PB15(MOSI). */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		      GPIO_SPI2_SCK | GPIO_SPI2_MOSI);
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT,
		      GPIO_SPI2_MISO);
	/* Enable CS pin on PB12. */
	gpio_set(GPIOB, GPIO12);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_10_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);

	/* Force to SPI mode. This should be default after reset! */
	SPI2_I2SCFGR = 0;
	spi_init_master(SPI2,
			SPI_CR1_BAUDRATE_FPCLK_DIV_256,
			SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
			SPI_CR1_CPHA_CLK_TRANSITION_2,
			SPI_CR1_DFF_8BIT,
			SPI_CR1_MSBFIRST);
	/* Ignore the stupid NSS pin. */
	spi_enable_software_slave_management(SPI2);
	spi_set_nss_high(SPI2);
	spi_enable(SPI2);

	(void)accel_read(ADXL345_DEVID);
	accel_write(ADXL345_POWER_CTL, ADXL345_POWER_CTL_MEASURE);
	accel_write(ADXL345_DATA_FORMAT, ADXL345_DATA_FORMAT_LALIGN);

	/* USB_DETECT as input. */
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO8);

	/* Green LED off, as output. */
	gpio_set(GPIOC, GPIO2);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);
}

void usbhid_target_usbd_after_init_and_before_first_poll(void)
{
	unsigned i;

		/* Delay some seconds to show that pull-up switch works. */
	for (i = 0; i < 0x800000; i++) {
		__asm__("nop");
	}

	/* Wait for USB Vbus. */
	while (gpio_get(GPIOA, GPIO8) == 0) {
		__asm__("nop");
	}

	/* Green LED on, connect USB. */
	gpio_clear(GPIOC, GPIO2);
	// OTG_FS_GCCFG &= ~OTG_FS_GCCFG_VBUSBSEN;
}

void
usbhid_target_accel_get(int16_t *x, int16_t *y, int16_t *z)
{
	(void)z;

	accel_get(x, y, NULL);

	if (x != NULL) {
		*x >>= 9;
	}

	if (y != NULL) {
		*y >>= 9;
	}
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
