/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This library is free software: you can redistribute it and/or modify * it under the terms of the GNU Lesser General Public License as published by * the Free Software Foundation, either version 3 of the License, or * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unicore-mx/nrf/gpio.h>

static void gpio_setup(void)
{
  /* Setup GPIOPIN19 (LED) */
  /* Using API functions: */
  gpio_set_mode(GPIO, GPIO_DIR_OUTPUT, GPIO_PUPD_NONE, GPIO19);
}

int main(void)
{
  int i;

  gpio_setup();

  /* Blink the LED (GPIOPIN19) on the board. */
  while (1) {

    /* Using API function gpio_toggle(): */
    gpio_toggle(GPIO, GPIO19);  // LED on/off
    for (i = 0; i < 800000; i++)  // Wait a bit.
      __asm__("nop");
  }

  return 0;
}

