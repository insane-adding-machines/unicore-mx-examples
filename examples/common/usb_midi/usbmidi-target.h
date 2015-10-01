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

#ifndef USBMIDI_TARGET_H
#define USBMIDI_TARGET_H

#include <stdbool.h>
#include <unicore-mx/usbd/usbd.h>

/* these functions are provided by target */

extern void usbmidi_target_init(void);
extern const usbd_backend *usbmidi_target_usb_driver(void);

/* return 0x01 on high, 0x00 for low */
extern uint32_t usbmidi_target_button_state(void);

extern void usbmidi_target_data_rx_cb(void);

#endif
