/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2015 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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

#ifndef USBDFU_TARGET_H
#define USBDFU_TARGET_H

#include <unicore-mx/usbd/usbd.h>

/* these functions are provided by target */

extern void usbdfu_target_init(void);
extern const usbd_backend *usbdfu_target_usb_driver(void);

/* see main() in usbdfu.c for more on this */
extern void usbdfu_target_usbd_after_init_and_before_first_poll(void);

/* flash operations */
void usbdfu_target_flash_lock(void);
void usbdfu_target_flash_unlock(void);
void usbdfu_target_flash_erase_page(uint32_t page);
void usbdfu_target_flash_write(uint32_t baseaddr, uint8_t *buf, uint16_t len);

#endif
