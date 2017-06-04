/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2013 Weston Schmidt <weston_schmidt@alumni.purdue.edu>
 * Copyright (C) 2013 Pavol Rusnak <stick@gk2.sk>
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

#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usbd/class/msc.h>

#include "ramdisk.h"
#include "msc-target.h"

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
	.bLength = USB_DT_STRING_SIZE(8),
	.bDescriptorType = USB_DT_STRING,
	/* MSC Demo */
	.wData = {
		0x004d, 0x0053, 0x0043, 0x0020, 0x0044, 0x0065, 0x006d, 0x006f
	}
};

static const struct usb_string_descriptor string_2 = {
	.bLength = USB_DT_STRING_SIZE(4),
	.bDescriptorType = USB_DT_STRING,
	/* DEMO */
	.wData = {
		0x0044, 0x0045, 0x004d, 0x004f
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

static const struct __attribute__((packed)) {
	struct usb_config_descriptor desc;
	struct usb_interface_descriptor msc_iface;
	struct usb_endpoint_descriptor msc_endp[2];
} config_descr = {
	.desc = {
		.bLength = USB_DT_CONFIGURATION_SIZE,
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(config_descr),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0x80,
		.bMaxPower = 0x32
	},

	.msc_iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_CLASS_MSC,
		.bInterfaceSubClass = USB_MSC_SUBCLASS_SCSI,
		.bInterfaceProtocol = USB_MSC_PROTOCOL_BBB,
		.iInterface = 0
	},

	.msc_endp = {{
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x01,
		.bmAttributes = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize = 64,
		.bInterval = 0,
	}, {
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x82,
		.bmAttributes = USB_ENDPOINT_ATTR_BULK,
		.wMaxPacketSize = 64,
		.bInterval = 0,
	}}
};

static const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0110,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5741,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1
};

static const struct usbd_info info = {
	.device = {
		.desc = &dev_descr,
		.string = &string
	},

	.config = {{
		.desc = (const struct usb_config_descriptor *) &config_descr,
		.string = &string
	}}
};

static usbd_msc *ms;

static void set_config_callback(usbd_device *usbd_dev,
			const struct usb_config_descriptor *cfg)
{
	(void) cfg;

	usbd_ep_prepare(usbd_dev, 0x01, USBD_EP_BULK, 64, USBD_INTERVAL_NA,
								USBD_EP_DOUBLE_BUFFER);
	usbd_ep_prepare(usbd_dev, 0x82, USBD_EP_BULK, 64, USBD_INTERVAL_NA,
								USBD_EP_DOUBLE_BUFFER);
	usbd_msc_start(ms);
}

static void setup_callback(usbd_device *usbd_dev, uint8_t ep_addr,
			const struct usb_setup_data *setup_data)
{
	(void) ep_addr; /* assuming ep_addr == 0 */

	if (!usbd_msc_setup_ep0(ms, setup_data)) {
		usbd_ep0_setup(usbd_dev, setup_data);
	}
}

int main(void)
{
	msc_target_init();

	usbd_device *msc_dev = usbd_init(msc_target_usb_driver(), NULL, &info);

	ramdisk_init();

	ms = usbd_msc_init(msc_dev, 0x82, 64, 0x01, 64, &ramdisk);
	usbd_register_set_config_callback(msc_dev, set_config_callback);
	usbd_register_setup_callback(msc_dev, setup_callback);

	for (;;) {
		usbd_poll(msc_dev, 0);
	}
}
