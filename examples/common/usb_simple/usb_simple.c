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

#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usbd/misc/string.h>
#include "usb_simple-target.h"

const struct usb_interface_descriptor iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = 0xFF,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
};

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &iface,
}};

const struct usb_config_descriptor config[] = {{
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
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
uint8_t usbd_control_buffer[128];

static enum usbd_control_result
simple_control_callback(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	(void)usbd_dev;

	if (arg->setup.bmRequestType != 0x40) {
		return USBD_REQ_NEXT; /* Only accept vendor request. */
	}

	usb_simple_led_set_value(!!(arg->setup.wValue & 1));

	return USBD_REQ_HANDLED;
}

int main(void)
{
	usbd_device *usbd_dev;

	usb_simple_target_init();

	usbd_dev = usbd_init(usb_simple_target_usb_driver(), &dev,
				usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_get_string_callback(usbd_dev, usb_strings);
	usbd_register_control_callback(usbd_dev, simple_control_callback);

	while (1) {
		usbd_poll(usbd_dev);
	}

	return 0;
}

