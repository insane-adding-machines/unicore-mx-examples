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

#ifndef USBHID_TARGET_H
#define USBHID_TARGET_H

#include <unicore-mx/usbd/usbd.h>

/* these functions are provided by target */

extern void usbhid_target_init(void);
extern const usbd_backend *usbhid_target_usb_driver(void);

/* see main() in usbhid.c for more on this */
extern void usbhid_target_usbd_after_init_and_before_first_poll(void);

/* Optional: there is a fake version */
extern void usbhid_target_accel_get(int16_t *x, int16_t *y, int16_t *z);

/* Optional */
extern void usbhid_detach_complete_before_scb_reset_core(void);

#endif
