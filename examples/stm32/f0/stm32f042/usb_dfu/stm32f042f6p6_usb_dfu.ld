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

/* Linker script for STM32F0 targets USB DFU bootloader. */

/* Define memory regions. */
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 32K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 6K
}

/* Define sections. */
SECTIONS
{
	/*
	 * In order to boot the board using a bootloader, the app interrupt
	 * vector table has to be remapped.
	 *
	 * Some STM32 chips have an interrupt vector offset register, so these
	 * chips can use the app's interrupt vector in-flash by setting this
	 * register properly.
	 *
	 * As STM32F0 does not support this feature, the app must copy its own
	 * vector table at the beginning of RAM. Before jumping into the app,
	 * bootloader maps the shadow memory area to RAM, so that vector table
	 * is used.
	 */
	.ram_nvic_table(NOLOAD): { KEEP (*(.ram_nvic_table)) } >ram
}

INCLUDE libucmx_stm32f0.ld
