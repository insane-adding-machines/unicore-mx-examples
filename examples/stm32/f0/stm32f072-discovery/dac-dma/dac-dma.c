/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2014 Ken Sarkies <ksarkies@internode.on.net>
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

#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/timer.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/dac.h>
#include <unicore-mx/stm32/dma.h>
#include <unicore-mx/stm32/flash.h>

/* Timer 2 count period, 16 microseconds for a 72MHz APB2 clock */
#define PERIOD 1152

/* Globals */
uint8_t waveform[256];

/*--------------------------------------------------------------------*/

static void clock_setup(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
}

/*--------------------------------------------------------------------*/
static void gpio_setup(void)
{
	/* Port A and C are on AHB1 */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	/* Set the digital test output on PC1 */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO1);
	/* Set PA4 for DAC channel 1 to analogue, ignoring drive mode. */
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);
}

/*--------------------------------------------------------------------*/
static void timer_setup(void)
{
	rcc_periph_clock_enable(RCC_TIM6);

	//timer_reset(TIM6);
	timer_set_period(TIM6, PERIOD);
	timer_set_prescaler(TIM6, 0);

	timer_set_master_mode(TIM6, TIM_CR2_MMS_UPDATE);
	timer_enable_counter(TIM6);
}

/*--------------------------------------------------------------------*/
static void dma_setup(void)
{
	/* DAC channel 1 uses DMA controller 1 Stream 5 Channel 7. */
	/* Enable DMA1 clock and IRQ */
	rcc_periph_clock_enable(RCC_DMA);

	dma_channel_reset(DMA1, DMA_CHANNEL3);

	dma_set_priority(DMA1, DMA_CHANNEL3, DMA_CCR_PL_HIGH);

	dma_set_memory_size(DMA1, DMA_CHANNEL3, DMA_CCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_CHANNEL3, DMA_CCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL3);
	dma_enable_circular_mode(DMA1, DMA_CHANNEL3);

	//dma_set_transfer_mode(DMA, DMA_CHANNEL3, DMA_CCR_DIR_MEM_TO_PERIPHERAL);
	//dma_set_read_from_peripheral(DMA1, DMA_CHANNEL3);
	dma_set_read_from_memory(DMA1, DMA_CHANNEL3);

	/* The register to target is the DAC1 8-bit right justified data register */
	dma_set_peripheral_address(DMA1, DMA_CHANNEL3, (uint32_t) &DAC_DHR8R1);
	/* The array v[] is filled with the waveform data to be output */
	dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t) waveform);
	dma_set_number_of_data(DMA1, DMA_CHANNEL3, 256);

	nvic_enable_irq(NVIC_DMA1_CHANNEL2_3_IRQ);
	dma_enable_transfer_complete_interrupt(DMA1, DMA_CHANNEL3);

	//dma_channel_select(DMA, DMA_CHANNEL3, DMA_CCR_CHSEL_7);

	dma_enable_channel(DMA1, DMA_CHANNEL3);
}

/*--------------------------------------------------------------------*/
static void dac_setup(void)
{
	/* Enable the DAC clock on APB1 */
	//rcc_periph_clock_enable(RCC_DAC);
	/* Setup the DAC channel 1, with timer 2 as trigger source.
	 * Assume the DAC has woken up by the time the first transfer occurs */
	//dac_trigger_enable(CHANNEL_1);
	//dac_set_trigger_source(DAC_CR_TSEL1_T6);
	//dac_dma_enable(CHANNEL_1);
	//dac_enable(CHANNEL_1);

	rcc_periph_clock_enable(RCC_DAC);

	dac_disable(CHANNEL_1);
	dac_set_waveform_characteristics(DAC_CR_MAMP1_12);

	//dac_set_waveform_generation(DAC_CR_WAVE1_TRI);

	dac_trigger_enable(CHANNEL_1);
	dac_set_trigger_source(DAC_CR_TSEL1_T6);

	dac_enable(CHANNEL_1);
}

/*--------------------------------------------------------------------*/
/* The ISR simply provides a test output for a CRO trigger */

void dma1_channel2_3_isr(void)
{
	if (dma_get_interrupt_flag(DMA1, DMA_CHANNEL3, DMA_TCIF))
	{
		dma_clear_interrupt_flags(DMA1, DMA_CHANNEL3, DMA_TCIF);
		/* Toggle PC1 just to keep aware of activity and frequency. */
		gpio_toggle(GPIOC, GPIO1);
	}
}

/*--------------------------------------------------------------------*/
int main(void)
{
	/* Fill the array with funky waveform data */
	/* This is for dual channel 8-bit right aligned */
	uint16_t i, x;
	for (i=0; i<256; i++)
	{
		if (i<10) x=10;
		else if (i<121) x=10+((i*i)>>7);
		else if (i<170) x=i/2;
		else if (i<246) x=i+(80-i/2);
		else x=10;
		waveform[i] = x;
	}
	clock_setup();
	gpio_setup();
	timer_setup();
	dma_setup();
	dac_setup();

	while (1);

	return 0;
}
