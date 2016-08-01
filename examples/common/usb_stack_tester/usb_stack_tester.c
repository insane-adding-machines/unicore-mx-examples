/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2011 Gareth McMullin <gareth@blacksphere.co.nz>
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

#include <stdlib.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usbd/misc/string.h>
#include "usb_stack_tester-target.h"

const struct usb_config_descriptor config[] = {{
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 0,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = NULL,
}};

const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xFF,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0xCAFE,
	.idProduct = 0xCAFE,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,

	.config = config
};

const char *usb_strings_ascii[] = {
	"Black Sphere Technologies",
	"Simple Device",
	"1001",
};

static int usb_strings(usbd_device *usbd_dev, struct usbd_get_string_arg *arg)
{
	(void)usbd_dev;
	return usbd_handle_string_ascii(arg, usb_strings_ascii, 3);
}

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[1024];

static enum usbd_control_result
simple_control_callback(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	(void)usbd_dev;

	if (arg->setup.bmRequestType != 0xC0) {
		return USBD_REQ_NEXT; /* Only accept vendor In request. */
	}

	/* just secretly send length what wValue has */
	arg->len = arg->setup.wValue;
	return USBD_REQ_HANDLED;
}

int main(void)
{
	usbd_device *usbd_dev;

	usb_stack_tester_target_init();

	usbd_dev = usbd_init(usb_stack_tester_target_usb_driver(),
		&dev, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_get_string_callback(usbd_dev, usb_strings);
	usbd_register_control_callback(usbd_dev, simple_control_callback);

	while (1) {
		usbd_poll(usbd_dev);
	}
}

