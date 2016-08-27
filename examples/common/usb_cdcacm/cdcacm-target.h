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

#ifndef CDCACM_TARGET_H
#define CDCACM_TARGET_H

#include <unicore-mx/usbd/usbd.h>

/* these functions are provided by target */

extern void cdcacm_target_init(void);
extern const usbd_backend *cdcacm_target_usb_driver(void);

extern const usbd_backend_config * cdcacm_target_usb_config(void);

/* see main() in cdcacm.c for more on this */
extern void cdcacm_target_usbd_after_init_and_before_first_poll(void);

/* see cdcacm_data_rx_cb() in cdcacm.c for more on this */
extern void cdcacm_target_data_rx_cb_before_return(void);

#endif
