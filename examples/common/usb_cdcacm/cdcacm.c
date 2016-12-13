/*
 * This file is part of the unicore-mx project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
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

#include <string.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>
#include "cdcacm-target.h"

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 },
};

static const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extra_len = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
}};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
}};

const uint8_t *usb_strings_english[] = {
	(uint8_t *) "Black Sphere Technologies",
	(uint8_t *) "CDC-ACM Demo",
	(uint8_t *) "DEMO",
};

/*
 * Black = काला
 * Sphere = गोला
 * Technologies = प्रौद्योगिकी
 * CDC-ACM = सीडीसी-एसीएम
 * Demo, DEMO = नमूना
 */
const uint8_t *usb_strings_hindi[] = {
	(uint8_t *) "काला गोला प्रौद्योगिकी",
	(uint8_t *) "सीडीसी-एसीएम नमूना",
	(uint8_t *) "नमूना"
};

const struct usb_string_utf8_data usb_strings[] = {{
	.data = usb_strings_english,
	.count = 3,
	.lang_id = USB_LANGID_ENGLISH_UNITED_STATES
}, {
	.data = usb_strings_hindi,
	.count = 3,
	.lang_id = USB_LANGID_HINDI
}, {
	.data = NULL
}};

static const struct usb_config_descriptor config[] = {{
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 2,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
	.string = usb_strings
}};

static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_CDC,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5740,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,

	.config = config,
	.string = usb_strings
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128] __attribute__((aligned(2)));

static void cdcacm_control_request(usbd_device *usbd_dev, uint8_t ep,
				const struct usb_setup_data *setup_data)
{
	(void) ep; /* assuming ep == 0 */

	const uint8_t bmReqMask = USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT;
	const uint8_t bmReqVal = USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE;

	if ((setup_data->bmRequestType & bmReqMask) != bmReqVal) {
		/* Pass on to usb stack internal */
		usbd_ep0_setup(usbd_dev, setup_data);
		return;
	}

	switch (setup_data->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		usbd_ep0_transfer(usbd_dev, setup_data, NULL, 0, NULL);
	return;
	case USB_CDC_REQ_SET_LINE_CODING:
		if (setup_data->wLength < sizeof(struct usb_cdc_line_coding)) {
			break;
		}

		/* Just read what ever host is sending and do the status stage */
		usbd_ep0_transfer(usbd_dev, setup_data, usbd_control_buffer,
			setup_data->wLength, NULL);
	return;
	}

	usbd_ep0_stall(usbd_dev);
}

void __attribute__((weak))
cdcacm_target_data_rx_cb_before_return(void) { /* empty */ }

static uint8_t bulk_buf[64];

static void cdcacm_data_rx_cb(usbd_device *usbd_dev,
	const usbd_transfer *_transfer, usbd_transfer_status status,
	usbd_urb_id urb_id);

static void cdcacm_data_tx_cb(usbd_device *usbd_dev,
	const usbd_transfer *_transfer, usbd_transfer_status status,
	usbd_urb_id urb_id);

static void rx_from_host(usbd_device *usbd_dev)
{
	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = 0x01,
		.ep_size = 64,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = bulk_buf,
		.length = 64,
		.flags = USBD_FLAG_SHORT_PACKET,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = cdcacm_data_rx_cb
	};

	usbd_transfer_submit(usbd_dev, &transfer);
}

static void tx_to_host(usbd_device *usbd_dev, void *data, size_t len)
{
	const usbd_transfer transfer = {
		.ep_type = USBD_EP_BULK,
		.ep_addr = 0x82,
		.ep_size = 64,
		.ep_interval = USBD_INTERVAL_NA,
		.buffer = data,
		.length = len,
		.flags = USBD_FLAG_SHORT_PACKET,
		.timeout = USBD_TIMEOUT_NEVER,
		.callback = cdcacm_data_tx_cb
	};

	usbd_transfer_submit(usbd_dev, &transfer);
}

static void cdcacm_data_tx_cb(usbd_device *usbd_dev,
	const usbd_transfer *transfer, usbd_transfer_status status,
	usbd_urb_id urb_id)
{
	(void) urb_id;
	(void) transfer;

	if (status == USBD_SUCCESS) {
		rx_from_host(usbd_dev);
	}
}

static void cdcacm_data_rx_cb(usbd_device *usbd_dev,
	const usbd_transfer *transfer, usbd_transfer_status status,
	usbd_urb_id urb_id)
{
	(void) urb_id;

	if (status == USBD_SUCCESS) {
		if (transfer->transferred) {
			tx_to_host(usbd_dev, transfer->buffer, transfer->transferred);
		} else {
			usbd_transfer_submit(usbd_dev, transfer); /* re-submit */
		}
	}

	/* this was only found in f1/lisa-m-1/usb_cdcacm.c */
	cdcacm_target_data_rx_cb_before_return();
}

static void cdcacm_set_config(usbd_device *usbd_dev,
				const struct usb_config_descriptor *cfg)
{
	(void)cfg;
	usbd_ep_prepare(usbd_dev, 0x01, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
	usbd_ep_prepare(usbd_dev, 0x82, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
	usbd_ep_prepare(usbd_dev, 0x83, USBD_EP_INTERRUPT, 16, USBD_INTERVAL_NA, USBD_EP_NONE);

	rx_from_host(usbd_dev);
}

void __attribute__((weak))
cdcacm_target_usbd_after_init_and_before_first_poll(void) { /* empty */ }

const usbd_backend_config * __attribute__((weak))
cdcacm_target_usb_config(void) { return NULL; }

int main(void)
{
	usbd_device *usbd_dev;

	cdcacm_target_init();

	usbd_dev = usbd_init(cdcacm_target_usb_driver(),
		cdcacm_target_usb_config(), &dev,
		usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);
	usbd_register_setup_callback(usbd_dev, cdcacm_control_request);

	/* on f3, here was a busy loop after usbd init and before first poll.
	 *  as the use of the busy loop is unknow,
	 *  retaining the code for compatibility */
	cdcacm_target_usbd_after_init_and_before_first_poll();

	while (1) {
		usbd_poll(usbd_dev, 0);
	}
}
