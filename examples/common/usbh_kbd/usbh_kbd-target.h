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

#ifndef USBH_KBD_TARGET_H
#define USBH_KBD_TARGET_H

#include <unicore-mx/usbh/usbh.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

void usbh_kbd_init(void);

usbh_backend *usbh_kbd_backend(void);

/**
 * The timer is expected to
 *  - count to up
 *  - counting at 10Khz
 *  - roll over to 0 on overflow
 */
uint16_t tim_get_counter(void);

void usbh_kbd_before_poll(void);
void usbh_kbd_after_poll(void);

void usart_puts(const char *arg);
void usart_printf(const char *fmt, ...);
void usart_vprintf(const char *fmt, va_list va);

#endif
