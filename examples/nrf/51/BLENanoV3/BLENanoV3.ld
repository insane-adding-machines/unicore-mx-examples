/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
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

/* Linker script for BLENano (256K flash, 16K RAM). */

/* Define memory regions. */

MEMORY
{
  rom (rx) : ORIGIN = 0x0, LENGTH = 0x40000
  ram (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x8000
}

/* Include the common ld script. */
INCLUDE libucmx_nrf51.ld
