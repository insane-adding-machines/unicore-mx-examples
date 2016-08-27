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

static const struct usb_endpoint_descriptor msc_endp[] = {{
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
}};

static const struct usb_interface_descriptor msc_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_MSC,
	.bInterfaceSubClass = USB_MSC_SUBCLASS_SCSI,
	.bInterfaceProtocol = USB_MSC_PROTOCOL_BBB,
	.iInterface = 0,
	.endpoint = msc_endp,
	.extra = NULL,
	.extra_len = 0
}};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = msc_iface,
}};

static const uint8_t *usb_strings_ascii[] = {
	(uint8_t *) "Black Sphere Technologies",
	(uint8_t *) "MSC Demo",
	(uint8_t *) "DEMO",
};

const struct usb_string_utf8_data usb_strings[] = {{
	.data = usb_strings_ascii,
	.count = 3,
	.lang_id = USB_LANGID_ENGLISH_UNITED_STATES
}, {
	.data = NULL
}};

static const struct usb_config_descriptor config_descr[] = {{
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
	.bNumConfigurations = 1,

	.config = config_descr,
	.string = usb_strings
};

/* Buffer to be used for setup requests. */
static uint8_t usbd_control_buffer[128];

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

	usbd_device *msc_dev = usbd_init(msc_target_usb_driver(), NULL,
				&dev_descr, usbd_control_buffer, sizeof(usbd_control_buffer));

	ramdisk_init();

	ms = usbd_msc_init(msc_dev, 0x82, 64, 0x01, 64, &ramdisk);
	usbd_register_set_config_callback(msc_dev, set_config_callback);
	usbd_register_setup_callback(msc_dev, setup_callback);

	for (;;) {
		usbd_poll(msc_dev, 0);
	}
}
