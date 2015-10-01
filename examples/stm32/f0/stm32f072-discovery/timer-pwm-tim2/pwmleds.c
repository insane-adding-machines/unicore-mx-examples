/*
 * This file is part of the unicore-mx project.
 *
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
#include <unicore-mx/stm32/flash.h>

static void clock_setup(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();

	/* Enable TIM2 clock. */
	rcc_periph_clock_enable(RCC_TIM2);

	/* Enable GPIOB, Alternate Function clocks. */
	rcc_periph_clock_enable(RCC_GPIOB);
}

static void gpio_setup(void)
{
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO10);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO10); // 100mhz means highspeed actually
	gpio_set_af(GPIOB, GPIO_AF2, GPIO10);
}

static void tim_setup(void)
{
	//timer_reset(TIM2);
	timer_set_prescaler(TIM2, 0xFF);
	//timer_set_clock_division(TIM2, 0xFF);
	timer_set_period(TIM2, 0xFFF);

	timer_continuous_mode(TIM2);
	timer_direction_up(TIM2);

	timer_disable_oc_output(TIM2, TIM_OC3);
	timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM1);
	timer_set_oc_value(TIM2, TIM_OC3, 0xFF);
	timer_enable_oc_output(TIM2, TIM_OC3);
	timer_enable_preload(TIM2);
	timer_enable_counter(TIM2);
}

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();

	while(1);
	return 0;
}
