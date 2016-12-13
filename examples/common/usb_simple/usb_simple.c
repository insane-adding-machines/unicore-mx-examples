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

const uint8_t *usb_strings_ascii[] = {
	(uint8_t *) "Black Sphere Technologies",
	(uint8_t *) "Simple Device",
	(uint8_t *) "1001",
};

const struct usb_string_utf8_data usb_strings[] = {{
	.data = usb_strings_ascii,
	.count = 3,
	.lang_id = USB_LANGID_ENGLISH_UNITED_STATES
}, {
	.data = NULL
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
	.string = usb_strings
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

	.config = config,
	.string = usb_strings
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

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

	usbd_dev = usbd_init(usb_simple_target_usb_driver(), NULL, &dev,
				usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_setup_callback(usbd_dev, simple_setup_callback);

	while (1) {
		usbd_poll(usbd_dev, 0);
	}

	return 0;
}

