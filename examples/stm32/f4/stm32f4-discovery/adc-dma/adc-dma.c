/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2018 Ariel D'Alessandro <ariel@vanguardiasur.com.ar>
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

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/adc.h>
#include <unicore-mx/stm32/dma.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/usart.h>

#define ADC_ID			ADC1
#define ADC_CHANNEL_LEN		2

static uint32_t adc_data[ADC_CHANNEL_LEN];

/* DMA2 Stream 0 Channel 0: ADC1. */
#define DMA_ID			DMA2
#define DMA_STREAM_ID		DMA_STREAM0
#define DMA_NVIC		NVIC_DMA2_STREAM0_IRQ
#define DMA_CHANNEL_ID		DMA_SxCR_CHSEL_0

#define LED_GREEN_PORT		GPIOD
#define LED_GREEN_PIN		GPIO12

#define USART_CONSOLE		USART2

int _write(int file, char *ptr, int len);

static void clock_setup(void)
{
	rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

	/* Enable GPIOA clock for USART2 and ADC1. */
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Enable GPIOD clock for LED. */
	rcc_periph_clock_enable(RCC_GPIOD);

	rcc_periph_clock_enable(RCC_ADC1);
	rcc_periph_clock_enable(RCC_DMA2);
	rcc_periph_clock_enable(RCC_USART2);
}

static void usart_setup(void)
{
	/* Setup USART2 TX pin. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

	usart_set_baudrate(USART_CONSOLE, 115200);
	usart_set_databits(USART_CONSOLE, 8);
	usart_set_stopbits(USART_CONSOLE, USART_STOPBITS_1);
	usart_set_mode(USART_CONSOLE, USART_MODE_TX);
	usart_set_parity(USART_CONSOLE, USART_PARITY_NONE);
	usart_set_flow_control(USART_CONSOLE, USART_FLOWCONTROL_NONE);

	usart_enable(USART_CONSOLE);
}

/* Use USART_CONSOLE as a console. This is a syscall for newlib. */
int _write(int file, char *ptr, int len)
{
	int i;

	if (file == STDOUT_FILENO || file == STDERR_FILENO) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n')
				usart_send_blocking(USART_CONSOLE, '\r');
			usart_send_blocking(USART_CONSOLE, ptr[i]);
		}
		return i;
	}

	errno = EIO;
	return -1;
}

void adc_isr(void)
{
	if (adc_get_overrun_flag(ADC_ID)) {
		/*
		 * DMA transfers are disabled after an overrun and DMA requests
		 * are no longer accepted.
		 * TODO: re-initialize both DMA and ADC.
		 */
		printf("%s: OVR flag - overrun.\n", __func__);
		adc_clear_overrun_flag(ADC_ID);
	}
}

static void adc_setup(void)
{
	/* ADC regular group of channels to convert: channels 0 and 1 */
	uint8_t channel_sequence[ADC_CHANNEL_LEN] = { 0, 1 };

	adc_power_off(ADC_ID);

	/* Setup pin: PA0: ADC1 channel 0. */
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	/* Setup pin: PA1: ADC1 channel 1. */
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

	nvic_enable_irq(NVIC_ADC_IRQ);
	adc_enable_overrun_interrupt(ADC_ID);

	adc_set_regular_sequence(ADC_ID, ADC_CHANNEL_LEN, channel_sequence);
	adc_set_sample_time_on_all_channels(ADC_ID, ADC_SMPR_SMP_144CYC);
	adc_set_right_aligned(ADC_ID);

	/* Perform a conversion for each channel in group. */
	adc_enable_scan_mode(ADC_ID);
	/* ADC starts a new conversion as soon as it finishes one. */
	adc_set_continuous_conversion_mode(ADC_ID);
	/* Continue generating DMA requests after last conversion. */
	adc_set_dma_continue(ADC_ID);
	adc_enable_dma(ADC_ID);

	/* Start DMA and kick conversions. */
	adc_power_on(ADC_ID);
	adc_start_conversion_regular(ADC_ID);
}

void dma2_stream0_isr(void)
{
	if (dma_get_interrupt_flag(DMA_ID, DMA_STREAM_ID, DMA_TEIF)) {
		printf("%s: TEIF flag - transfer error\n", __func__);
		dma_clear_interrupt_flags(DMA_ID, DMA_STREAM_ID, DMA_TEIF);
	}

	if (dma_get_interrupt_flag(DMA_ID, DMA_STREAM_ID, DMA_FEIF)) {
		printf("%s: FEIF flag - FIFO error\n", __func__);
		dma_clear_interrupt_flags(DMA_ID, DMA_STREAM_ID, DMA_FEIF);
	}
}

static void dma_setup(void)
{
	dma_disable_stream(DMA_ID, DMA_STREAM_ID);

	dma_stream_reset(DMA_ID, DMA_STREAM_ID);
	dma_channel_select(DMA_ID, DMA_STREAM_ID, DMA_CHANNEL_ID);
	dma_set_priority(DMA_ID, DMA_STREAM_ID, DMA_SxCR_PL_LOW);
	dma_set_transfer_mode(DMA_ID, DMA_STREAM_ID,
			      DMA_SxCR_DIR_PERIPHERAL_TO_MEM);

	/* Peripheral data is copied from ADC regular data register (ADC_DR). */
	dma_set_peripheral_address(DMA_ID, DMA_STREAM_ID,
				   (uint32_t)&ADC_DR(ADC_ID));
	dma_set_peripheral_size(DMA_ID, DMA_STREAM_ID,
				DMA_SxCR_PSIZE_32BIT);
	dma_disable_peripheral_increment_mode(DMA_ID, DMA_STREAM_ID);

	/* Each ADC channel data is stored sequentially in the array. */
	dma_set_memory_address(DMA_ID, DMA_STREAM_ID, (uint32_t)adc_data);
	dma_set_memory_size(DMA_ID, DMA_STREAM_ID, DMA_SxCR_MSIZE_32BIT);
	dma_enable_memory_increment_mode(DMA_ID, DMA_STREAM_ID);

	/* The number of items to transfer is fixed and set into the DMA. */
	dma_set_dma_flow_control(DMA_ID, DMA_STREAM_ID);
	dma_set_number_of_data(DMA_ID, DMA_STREAM_ID, ADC_CHANNEL_LEN);

	dma_enable_fifo_mode(DMA_ID, DMA_STREAM_ID);
	dma_set_fifo_threshold(DMA_ID, DMA_STREAM_ID, DMA_SxFCR_FTH_4_4_FULL);
	dma_enable_circular_mode(DMA_ID, DMA_STREAM_ID);

	dma_enable_transfer_error_interrupt(DMA_ID, DMA_STREAM_ID);
	dma_enable_fifo_error_interrupt(DMA_ID, DMA_STREAM_ID);
	nvic_enable_irq(DMA_NVIC);

	dma_enable_stream(DMA_ID, DMA_STREAM_ID);
}

static void led_setup(void)
{
	gpio_mode_setup(LED_GREEN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			LED_GREEN_PIN);
}

static inline void delay(int len)
{
	int i;

	for (i = 0; i < len; i++)
		__asm__("NOP");
}

int main(void)
{
	clock_setup();
	adc_setup();
	dma_setup();
	led_setup();
	usart_setup();

	printf("Starting adc-dma example!\n");

	while (1) {
		printf("ADC1: ch0=%lu, ch1=%lu\n", adc_data[0], adc_data[1]);
		gpio_toggle(LED_GREEN_PORT, LED_GREEN_PIN);
		delay(1000000);
	}

	return 0;
}
