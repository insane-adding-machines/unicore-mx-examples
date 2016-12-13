/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2011 Piotr Esden-Tempski <piotr@esden.net>
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

#include <stdlib.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/cm3/systick.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/hid.h>
#include "usbhid-target.h"

/* Define this (in the Makefile CFLAGS) to include the DFU APP interface. */
#ifdef INCLUDE_DFU_INTERFACE
#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/usb/class/dfu.h>
#endif

static usbd_device *usbd_dev;

static const uint8_t hid_report_descriptor[] = {
	0x05, 0x01, /* USAGE_PAGE (Generic Desktop)         */
	0x09, 0x02, /* USAGE (Mouse)                        */
	0xa1, 0x01, /* COLLECTION (Application)             */
	0x09, 0x01, /*   USAGE (Pointer)                    */
	0xa1, 0x00, /*   COLLECTION (Physical)              */
	0x05, 0x09, /*     USAGE_PAGE (Button)              */
	0x19, 0x01, /*     USAGE_MINIMUM (Button 1)         */
	0x29, 0x03, /*     USAGE_MAXIMUM (Button 3)         */
	0x15, 0x00, /*     LOGICAL_MINIMUM (0)              */
	0x25, 0x01, /*     LOGICAL_MAXIMUM (1)              */
	0x95, 0x03, /*     REPORT_COUNT (3)                 */
	0x75, 0x01, /*     REPORT_SIZE (1)                  */
	0x81, 0x02, /*     INPUT (Data,Var,Abs)             */
	0x95, 0x01, /*     REPORT_COUNT (1)                 */
	0x75, 0x05, /*     REPORT_SIZE (5)                  */
	0x81, 0x01, /*     INPUT (Cnst,Ary,Abs)             */
	0x05, 0x01, /*     USAGE_PAGE (Generic Desktop)     */
	0x09, 0x30, /*     USAGE (X)                        */
	0x09, 0x31, /*     USAGE (Y)                        */
	0x09, 0x38, /*     USAGE (Wheel)                    */
	0x15, 0x81, /*     LOGICAL_MINIMUM (-127)           */
	0x25, 0x7f, /*     LOGICAL_MAXIMUM (127)            */
	0x75, 0x08, /*     REPORT_SIZE (8)                  */
	0x95, 0x03, /*     REPORT_COUNT (3)                 */
	0x81, 0x06, /*     INPUT (Data,Var,Rel)             */
	0xc0,       /*   END_COLLECTION                     */
	0x09, 0x3c, /*   USAGE (Motion Wakeup)              */
	0x05, 0xff, /*   USAGE_PAGE (Vendor Defined Page 1) */
	0x09, 0x01, /*   USAGE (Vendor Usage 1)             */
	0x15, 0x00, /*   LOGICAL_MINIMUM (0)                */
	0x25, 0x01, /*   LOGICAL_MAXIMUM (1)                */
	0x75, 0x01, /*   REPORT_SIZE (1)                    */
	0x95, 0x02, /*   REPORT_COUNT (2)                   */
	0xb1, 0x22, /*   FEATURE (Data,Var,Abs,NPrf)        */
	0x75, 0x06, /*   REPORT_SIZE (6)                    */
	0x95, 0x01, /*   REPORT_COUNT (1)                   */
	0xb1, 0x01, /*   FEATURE (Cnst,Ary,Abs)             */
	0xc0        /* END_COLLECTION                       */
};

static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength = sizeof(hid_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0100,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	}
};

const struct usb_endpoint_descriptor hid_endpoint = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 4,
	.bInterval = 0x02,
};

const struct usb_interface_descriptor hid_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = 1, /* boot */
	.bInterfaceProtocol = 2, /* mouse */
	.iInterface = 0,

	.endpoint = &hid_endpoint,

	.extra = &hid_function,
	.extra_len = sizeof(hid_function),
};

#ifdef INCLUDE_DFU_INTERFACE
const struct usb_dfu_descriptor dfu_function = {
	.bLength = sizeof(struct usb_dfu_descriptor),
	.bDescriptorType = DFU_FUNCTIONAL,
	.bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
	.wDetachTimeout = 255,
	.wTransferSize = 1024,
	.bcdDFUVersion = 0x011A,
};

const struct usb_interface_descriptor dfu_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = 0xFE,
	.bInterfaceSubClass = 1,
	.bInterfaceProtocol = 1,
	.iInterface = 0,

	.extra = &dfu_function,
	.extra_len = sizeof(dfu_function),
};
#endif

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &hid_iface,
#ifdef INCLUDE_DFU_INTERFACE
}, {
	.num_altsetting = 1,
	.altsetting = &dfu_iface,
#endif
}};

static const uint8_t *usb_strings_ascii[] = {
	(uint8_t *) "Black Sphere Technologies",
	(uint8_t *) "HID Demo",
	(uint8_t *) "DEMO",
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
#ifdef INCLUDE_DFU_INTERFACE
	.bNumInterfaces = 2,
#else
	.bNumInterfaces = 1,
#endif
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,

	.interface = ifaces,
	.string = usb_strings
}};

const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5710,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,

	.config = config,
	.string = usb_strings
};

/* Buffer used for control requests. */
uint8_t usbd_control_buffer[128];

static bool hid_setup_callback(usbd_device *dev,
				const struct usb_setup_data *setup_data)
{
	if(	(setup_data->bmRequestType != 0x81) ||
		(setup_data->bRequest != USB_REQ_GET_DESCRIPTOR) ||
		(setup_data->wValue != 0x2200)) {
		return false;
	}

	/* Handle the HID report descriptor. */
	usbd_ep0_transfer(dev, setup_data, (void *) hid_report_descriptor,
			sizeof(hid_report_descriptor), NULL);
	return true;
}

#ifdef INCLUDE_DFU_INTERFACE

void __attribute__((weak))
usbhid_detach_complete_before_scb_reset_core(void) { /* empty */ }

static void dfu_detach_complete()
{
	usbhid_detach_complete_before_scb_reset_core();
	scb_reset_core();
}

bool dfu_control_request(usbd_device *dev,
					const struct usb_setup_data *setup_data)
{
	(void)dev;

	if ((setup_data->bmRequestType != 0x21) ||
		(setup_data->bRequest != DFU_DETACH)) {
		return false; /* Only accept class request. */
	}

	/* no data stage - dfu_detach_complete will be called in status stage */
	usbd_ep0_transfer(dev, setup_data, NULL, 0,
		(usbd_transfer_control_handle_callback) dfu_detach_complete);
	return true;
}
#endif

static void setup_callback(usbd_device *dev, uint8_t ep_addr,
						const struct usb_setup_data *setup_data)
{
	(void) ep_addr; /* assuming ep_addr == 0 */

	if (hid_setup_callback(dev, setup_data)) {
		return;
	}

#ifdef INCLUDE_DFU_INTERFACE
	if (hid_control_request(dev, arg)) {
		return;
	}
#endif

	usbd_ep0_setup(dev, setup_data);
}

static void hid_set_config(usbd_device *dev,
				const struct usb_config_descriptor *cfg)
{
	(void)cfg;

	usbd_ep_prepare(dev, 0x81, USBD_EP_INTERRUPT, 4, 2, USBD_EP_NONE);

#if defined(__ARM_ARCH_6M__)
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(99999*8);
#else
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(99999);
#endif
	systick_interrupt_enable();
	systick_counter_enable();
}

void __attribute__((weak))
usbhid_target_usbd_after_init_and_before_first_poll(void) { /* empty */ }

int main(void)
{
	usbhid_target_init();

	usbd_dev = usbd_init(usbhid_target_usb_driver(), NULL, &dev_descr,
		usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_set_config_callback(usbd_dev, hid_set_config);
	usbd_register_setup_callback(usbd_dev, setup_callback);

	usbhid_target_usbd_after_init_and_before_first_poll();

	while (1) {
		usbd_poll(usbd_dev, 0);
	}
}

/* Fake Version */
void __attribute__((weak))
usbhid_target_accel_get(int16_t *out_x, int16_t *out_y, int16_t *out_z)
{
	(void)out_y;
	(void)out_z;

	static int x = 0;
	static int dir = 1;

	if (out_x != NULL) {
		*out_x = dir;
	}

	x += dir;
	if (x > 30) {
		dir = -dir;
	}
	if (x < -30) {
		dir = -dir;
	}
}

static usbd_urb_id last_urb_id = USBD_INVALID_URB_ID;

static void transfer_callback(usbd_device *_usbd_dev,
	const usbd_transfer *transfer, usbd_transfer_status status,
	usbd_urb_id urb_id)
{
	(void) _usbd_dev;
	(void) transfer;
	(void) status;
	(void) urb_id;
	last_urb_id = USBD_INVALID_URB_ID;
}

void sys_tick_handler(void)
{
	if (last_urb_id != USBD_INVALID_URB_ID) {
		return;
	}

	int16_t x = 0, y = 0, z = 0;
	static uint8_t buf[4];

	usbhid_target_accel_get(&x, &y, &z);

	buf[0] = 0;
	buf[1] = x;
	buf[2] = y;
	buf[3] = z;

	const usbd_transfer transfer = {
		.ep_type = USBD_EP_INTERRUPT,
		.ep_addr = 0x81,
		.ep_size = 4,
		.ep_interval = 2,
		.buffer = buf,
		.length = 4,
		.flags = USBD_FLAG_NONE,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = transfer_callback,
	};

	last_urb_id = usbd_transfer_submit(usbd_dev, &transfer);
}
