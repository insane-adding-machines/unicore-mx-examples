/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2011 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2015, 2016 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
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
#include "usb_simple-target.h"

static const struct usb_string_descriptor string_lang_list = {
	.bLength = USB_DT_STRING_SIZE(1),
	.bDescriptorType = USB_DT_STRING,
	.wData = {
		USB_LANGID_ENGLISH_UNITED_STATES
	}
};

/* string descriptor string_[0..2] generated using usb-string.py */

static const struct usb_string_descriptor string_0 = {
	.bLength = USB_DT_STRING_SIZE(16),
	.bDescriptorType = USB_DT_STRING,
	/* Mad Resistor LLP */
	.wData = {
		0x004d, 0x0061, 0x0064, 0x0020, 0x0052, 0x0065, 0x0073, 0x0069,
		0x0073, 0x0074, 0x006f, 0x0072, 0x0020, 0x004c, 0x004c, 0x0050
	}
};

static const struct usb_string_descriptor string_1 = {
	.bLength = USB_DT_STRING_SIZE(13),
	.bDescriptorType = USB_DT_STRING,
	/* Simple Device */
	.wData = {
		0x0053, 0x0069, 0x006d, 0x0070, 0x006c, 0x0065, 0x0020, 0x0044,
		0x0065, 0x0076, 0x0069, 0x0063, 0x0065
	}
};

static const struct usb_string_descriptor string_2 = {
	.bLength = USB_DT_STRING_SIZE(4),
	.bDescriptorType = USB_DT_STRING,
	/* 1001 */
	.wData = {
		0x0031, 0x0030, 0x0030, 0x0031
	}
};

static const struct usb_string_descriptor **string_data[1] = {
	(const struct usb_string_descriptor *[]){&string_0, &string_1, &string_2},
};

static const struct usbd_info_string string = {
	.lang_list = &string_lang_list,
	.count = 3,
	.data = string_data
};

const struct __attribute__((packed)) {
	struct usb_config_descriptor config;
	struct usb_interface_descriptor iface;
} config_desc = {
	.config = {
		.bLength = USB_DT_CONFIGURATION_SIZE,
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(config_desc),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0x80,
		.bMaxPower = 0x32,
	},

	.iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = 0xFF,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0,
	}
};

const struct usb_device_descriptor dev_desc = {
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
	.bNumConfigurations = 1
};

static const struct usbd_info info = {
	.device = {
		.desc = &dev_desc,
		.string = &string
	},

	.config = {{
		.desc = (const struct usb_config_descriptor *) &config_desc,
		.string = &string
	}}
};

static void simple_setup_callback(usbd_device *usbd_dev, uint8_t ep_addr,
						const struct usb_setup_data *setup_data)
{
	(void) ep_addr; /* assuming ep_addr == 0 */

	if (setup_data->bmRequestType != 0x40) {
		usbd_ep0_setup(usbd_dev, setup_data); /* Only accept vendor request. */
		return;
	}

	usb_simple_led_set_value(!!(setup_data->wValue & 1));
	usbd_ep0_transfer(usbd_dev, setup_data, NULL, 0, NULL);
}

int main(void)
{
	usbd_device *usbd_dev;

	usb_simple_target_init();

	usbd_dev = usbd_init(usb_simple_target_usb_driver(), NULL, &info);

	usbd_register_setup_callback(usbd_dev, simple_setup_callback);

	while (1) {
		usbd_poll(usbd_dev, 0);
	}

	return 0;
}

