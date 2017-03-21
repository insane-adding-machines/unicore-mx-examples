/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2016, 2017 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
 * Copyright (C) 2015 Amir Hammad <amir.hammad@hotmail.com>
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

#include "usbh_kbd-target.h"

#include <unicore-mx/stm32/memorymap.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/usart.h>
#include <unicore-mx/stm32/timer.h>
#include <unicore-mx/stm32/otg_fs.h>
#include <unicore-mx/stm32/otg_hs.h>

/* #define USE_OTG_HS */

static void clock_setup(void)
{
	rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOH);

	rcc_periph_clock_enable(RCC_USART6);
#if defined(USE_OTG_HS)
	rcc_periph_clock_enable(RCC_OTGHS);
	rcc_periph_clock_enable(RCC_OTGHSULPI);
#else
	rcc_periph_clock_enable(RCC_OTGFS);
#endif
	rcc_periph_clock_enable(RCC_TIM6);
}

#if defined(USE_OTG_HS)
static void ulpi_pins(uint32_t gpioport, uint16_t gpiopins)
{
	gpio_mode_setup(gpioport, GPIO_MODE_AF, GPIO_PUPD_NONE, gpiopins);
	gpio_set_output_options(gpioport, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, gpiopins);
	gpio_set_af(gpioport, GPIO_AF10, gpiopins);
}
#endif

static void gpio_setup(void)
{
	/* Set GPIO12-15 (in GPIO port D) to 'output push-pull'. */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO12 | GPIO13 | GPIO14 | GPIO15);

	/* Set */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
	gpio_clear(GPIOC, GPIO0);

#if defined(USE_OTG_HS)
	/* OTG_HS */
	/* ULPI   GPIO
	 *  D0  -> PA3
	 *  D1  -> PB0
	 *  D2  -> PB1
	 *  D3  -> PB10
	 *  D4  -> PB11
	 *  D5  -> PB12
	 *  D6  -> PB13
	 *  D7  -> PB5
	 *  DIR -> PC2
	 *  STP -> PC0
	 *  NXT -> PH4
	 *  CK  -> PA5
	 */
	ulpi_pins(GPIOA, GPIO3 | GPIO5);
	ulpi_pins(GPIOB, GPIO0 | GPIO1 | GPIO5  | GPIO10 | GPIO11 | GPIO12 | GPIO13);
	ulpi_pins(GPIOC, GPIO0 | GPIO2);
	ulpi_pins(GPIOH, GPIO4);
#else /* defined(USE_OTG_HS) */
	/* OTG_FS */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);
#endif

	/* USART TX */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
	gpio_set_af(GPIOC, GPIO_AF8, GPIO6);
}

/**
 * Initalize the Output (TX) mode only (log)
 */
static void usart_init(void)
{
	usart_set_baudrate(USART6, 921600);
	usart_set_databits(USART6, 8);
	usart_set_flow_control(USART6, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART6, USART_MODE_TX);
	usart_set_parity(USART6, USART_PARITY_NONE);
	usart_set_stopbits(USART6, USART_STOPBITS_1);

	usart_enable(USART6);
}

/**
 * Setup the timer in 10Khz mode
 * This timer will be used to keep track of last poll time difference
 */
static void tim_setup(void)
{
	uint32_t tick_freq = 10000;

	uint32_t timer_clock = rcc_apb1_frequency;
	if (rcc_ahb_frequency != rcc_apb1_frequency) {
		timer_clock *= 2;
	}

	uint32_t clock_div = timer_clock / tick_freq;

	timer_reset(TIM6);
	timer_set_prescaler(TIM6, clock_div - 1);
	timer_set_period(TIM6, 0xFFFF);
	timer_enable_counter(TIM6);
}

/**
 * Get the counter value of Timer
 * @return Counter value
 */
uint16_t tim_get_counter(void)
{
	return timer_get_counter(TIM6);
}

/**
 * Output string @a arg
 */
void usart_puts(const char *arg)
{
	while (*arg != '\0') {
		usart_wait_send_ready(USART6);
		usart_send(USART6, *arg++);
	}
}

void usart_vprintf(const char *fmt, va_list va)
{
	char db[128];
	if (vsnprintf(db, sizeof(db), fmt, va) > 0) {
		usart_puts(db);
	}
}

void usart_printf(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	usart_vprintf(fmt, va);
	va_end(va);
}

void usbh_kbd_init(void)
{
	clock_setup();
	gpio_setup();

	tim_setup();

	usart_init();
}

usbh_backend *usbh_kbd_backend(void)
{
#if defined(USE_OTG_HS)
	return USBH_STM32_OTG_HS;
#else /* defined(USE_OTG_HS) */
	return USBH_STM32_OTG_FS;
#endif /* defined(USE_OTG_HS) */
}

#if defined(USE_OTG_HS)
static const usbh_backend_config _config = {
	.chan_count = 12,
	.priv_mem = 4096,
	.speed = USBH_SPEED_HIGH,
	/* VBUS_EXT = PHY IC drive VBUS */
	.feature = USBH_PHY_EXT | USBH_VBUS_EXT
};

const usbh_backend_config *usbh_kbd_config(void)
{
	return &_config;
}
#endif /* defined(USE_OTG_HS) */

void usbh_kbd_after_poll(void)
{
	unsigned i;

	/* dummy delay */
	for (i = 0; i < 14903; i++) {}
}
