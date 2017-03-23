/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 * Copyright (C) 2016 Google Inc
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

#include <unicore-mx/sam/gpio.h>
#include <unicore-mx/sam/scif.h>
#include <unicore-mx/sam/pm.h>

static void configure_clocks(void)
{
    scif_osc_enable(OSC_MODE_XIN_XOUT, 12 * 1000000, OSC_STARTUP_1K);
    // Configuring PLL to run at 96 MHz (div = 1, mul = 7).
	// No matter what I do, can't run it at higher frequency...
    scif_enable_pll(0x3f, 7, 1, 0, PLL_CLK_SRC_OSC0);

	pm_enable_clock_div(CKSEL_PBA, 1);
	pm_enable_clock_div(CKSEL_PBB, 1);
	pm_enable_clock_div(CKSEL_PBC, 1);
	pm_enable_clock_div(CKSEL_PBD, 1);
	pm_enable_clock_div(CKSEL_CPU, 1);

    pm_select_main_clock(MCK_SRC_PLL);
}

static void configure_pins(void)
{
    gpio_enable(GPIOC, GPIO10, GPIO_MODE_OUT);
}

int main(void)
{
    configure_clocks();
    configure_pins();

    while (1) {
        gpio_toggle(GPIOC, GPIO10);
        for (int i = 0; i < 1000*1000; ++i) {
            __asm__("nop");
        }
    }

    return 0;
}
