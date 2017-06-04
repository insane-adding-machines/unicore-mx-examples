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
#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/dfu.h>
#include "usbdfu-target.h"

/* Commands sent with wBlockNum == 0 as per ST implementation. */
/* FIXME: as per above not, this code is still infected with ST implementation */
#define CMD_SETADDR	0x21
#define CMD_ERASE	0x41

#define TRANSFER_SIZE 1024

static enum dfu_state usbdfu_state = STATE_DFU_IDLE;

static struct {
	uint8_t buf[TRANSFER_SIZE];
	uint16_t len;
	uint32_t addr;
	uint16_t blocknum;
} prog;

static const struct usb_string_descriptor string_lang_list = {
	.bLength = USB_DT_STRING_SIZE(2),
	.bDescriptorType = USB_DT_STRING,
	.wData = {
		USB_LANGID_ENGLISH_UNITED_STATES,
		USB_LANGID_HINDI
	}
};

/* string descriptor string_[0..5] generated using usb-string.py */

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
	/* DFU Demo */
	.wData = {
		0x0044, 0x0046, 0x0055, 0x0020, 0x0044, 0x0065, 0x006d, 0x006f
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

/*
 * Mad = पागल
 * Resistor = प्रतिरोधक
 * LLP = एलएलपी
 * DFU = डीएफयू
 * Demo, DEMO = नमूना
 */

static const struct usb_string_descriptor string_3 = {
	.bLength = USB_DT_STRING_SIZE(23),
	.bDescriptorType = USB_DT_STRING,
	/* पागल  प्रतिरोधक  एलएलपी */
	.wData = {
		0x092a, 0x093e, 0x0917, 0x0932, 0x0020, 0x0020, 0x092a, 0x094d,
		0x0930, 0x0924, 0x093f, 0x0930, 0x094b, 0x0927, 0x0915, 0x0020,
		0x0020, 0x090f, 0x0932, 0x090f, 0x0932, 0x092a, 0x0940
	}
};

static const struct usb_string_descriptor string_4 = {
	.bLength = USB_DT_STRING_SIZE(12),
	.bDescriptorType = USB_DT_STRING,
	/* डीएफयू नमूना */
	.wData = {
		0x0921, 0x0940, 0x090f, 0x092b, 0x092f, 0x0942, 0x0020, 0x0928,
		0x092e, 0x0942, 0x0928, 0x093e
	}
};

static const struct usb_string_descriptor string_5 = {
	.bLength = USB_DT_STRING_SIZE(5),
	.bDescriptorType = USB_DT_STRING,
	/* नमूना */
	.wData = {
		0x0928, 0x092e, 0x0942, 0x0928, 0x093e
	}
};

/* This string is used by ST Microelectronics' DfuSe utility. */
/* FIXME: this is ST Specific */
static const struct usb_string_descriptor string_st_flash_detail = {
	.bLength = USB_DT_STRING_SIZE(46),
	.bDescriptorType = USB_DT_STRING,
	/* @Internal Flash   /0x08000000/8*001Ka,56*001Kg */
	.wData = {
		0x0040, 0x0049, 0x006e, 0x0074, 0x0065, 0x0072, 0x006e, 0x0061,
		0x006c, 0x0020, 0x0046, 0x006c, 0x0061, 0x0073, 0x0068, 0x0020,
		0x0020, 0x0020, 0x002f, 0x0030, 0x0078, 0x0030, 0x0038, 0x0030,
		0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x002f, 0x0038, 0x002a,
		0x0030, 0x0030, 0x0031, 0x004b, 0x0061, 0x002c, 0x0035, 0x0036,
		0x002a, 0x0030, 0x0030, 0x0031, 0x004b, 0x0067
	}
};

static const struct usb_string_descriptor **string_data[2] = {
	(const struct usb_string_descriptor *[]){&string_0, &string_1, &string_2, &string_st_flash_detail},
	(const struct usb_string_descriptor *[]){&string_3, &string_4, &string_5, &string_st_flash_detail}
};

static const struct usbd_info_string string = {
	.lang_list = &string_lang_list,
	.count = 4,
	.data = string_data
};

static const struct __attribute__((packed)) {
	const struct usb_config_descriptor config;
	struct usb_interface_descriptor iface;
	const struct usb_dfu_descriptor dfu_function;
} config_desc = {
	.config = {
		.bLength = USB_DT_CONFIGURATION_SIZE,
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(config_desc),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0xC0,
		.bMaxPower = 0x32
	},

	.iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = 0xFE, /* Device Firmware Upgrade */
		.bInterfaceSubClass = 1,
		.bInterfaceProtocol = 2,

		/* The ST Microelectronics DfuSe application needs this string.
		 * The format isn't documented... */
		.iInterface = 4,
	},

	.dfu_function = {
		.bLength = sizeof(struct usb_dfu_descriptor),
		.bDescriptorType = DFU_FUNCTIONAL,
		.bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
		.wDetachTimeout = 255,
		.wTransferSize = TRANSFER_SIZE,
		.bcdDFUVersion = 0x011A,
	}
};

const struct usb_device_descriptor dev_desc = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0xDF11,
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

static uint8_t usbdfu_getstatus(uint32_t *bwPollTimeout)
{
	switch (usbdfu_state) {
	case STATE_DFU_DNLOAD_SYNC:
		usbdfu_state = STATE_DFU_DNBUSY;
		*bwPollTimeout = 100;
		return DFU_STATUS_OK;
	case STATE_DFU_MANIFEST_SYNC:
		/* Device will reset when read is complete. */
		usbdfu_state = STATE_DFU_MANIFEST;
		return DFU_STATUS_OK;
	default:
		return DFU_STATUS_OK;
	}
}

static usbd_control_transfer_feedback usbdfu_getstatus_complete(
		usbd_device *usbd_dev, const usbd_control_transfer_callback_arg *arg)
{
	(void) usbd_dev;
	(void) arg;

	if (arg != NULL) { /* Status Stage */
		return USBD_CONTROL_TRANSFER_OK;
	}

	switch (usbdfu_state) {
	case STATE_DFU_DNBUSY:
		usbdfu_target_flash_unlock();
		if (prog.blocknum == 0) {
			switch (prog.buf[0]) {
			case CMD_ERASE:
				{
					uint32_t *dat = (uint32_t *)(prog.buf + 1);
					usbdfu_target_flash_erase_page(*dat);
				}
			case CMD_SETADDR:
				{
					uint32_t *dat = (uint32_t *)(prog.buf + 1);
					prog.addr = *dat;
				}
			}
		} else {
			uint32_t baseaddr = prog.addr + ((prog.blocknum - 2) *
				       TRANSFER_SIZE);
			usbdfu_target_flash_write(baseaddr, prog.buf, prog.len);
		}
		usbdfu_target_flash_lock();

		/* Jump straight to dfuDNLOAD-IDLE, skipping dfuDNLOAD-SYNC. */
		usbdfu_state = STATE_DFU_DNLOAD_IDLE;
	break;
	case STATE_DFU_MANIFEST:
		/* USB device must detach, we just reset... */
		scb_reset_system();
	break; /* Will never return. */
	default:
	break;
	}

	return true;
}

static void usbdfu_setup_callback(usbd_device *usbd_dev, uint8_t ep_addr,
				const struct usb_setup_data *setup_data)
{
	(void)usbd_dev;
	(void)ep_addr; /* assuming ep_addr == 0 */
	const uint8_t bmReqMask = USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT;
	const uint8_t bmReqVal = USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE;

	if ((setup_data->bmRequestType & bmReqMask) != bmReqVal) {
		usbd_ep0_setup(usbd_dev, setup_data);
		return;
	}

	switch (setup_data->bRequest) {
	case DFU_DNLOAD:
		if (!setup_data->wLength) {
			usbdfu_state = STATE_DFU_MANIFEST_SYNC;
			usbd_ep0_transfer(usbd_dev, setup_data, NULL, 0, NULL);
		} else {
			/* Copy download data for use on GET_STATUS. */
			prog.blocknum = setup_data->wValue;
			prog.len = setup_data->wLength;
			usbdfu_state = STATE_DFU_DNLOAD_SYNC; /* FIXME: move this to callback */
			usbd_ep0_transfer(usbd_dev, setup_data, prog.buf, setup_data->wLength, NULL);
		}
	return;
	case DFU_CLRSTATUS:
		/* Clear error and return to dfuIDLE. */
		if (usbdfu_state == STATE_DFU_ERROR) {
			usbdfu_state = STATE_DFU_IDLE;
		}
		usbd_ep0_transfer(usbd_dev, setup_data, NULL, 0, NULL);
	return;
	case DFU_ABORT:
		/* Abort returns to dfuIDLE state. */
		usbdfu_state = STATE_DFU_IDLE;
		usbd_ep0_transfer(usbd_dev, setup_data, NULL, 0, NULL);
	return;
	case DFU_UPLOAD:
		/* Upload not supported for now. */
	break;
	case DFU_GETSTATUS: {
		uint32_t bwPollTimeout = 0; /* 24-bit integer in DFU class spec */
		static uint8_t buf[6];
		buf[0] = usbdfu_getstatus(&bwPollTimeout);
		buf[1] = bwPollTimeout & 0xFF;
		buf[2] = (bwPollTimeout >> 8) & 0xFF;
		buf[3] = (bwPollTimeout >> 16) & 0xFF;
		buf[4] = usbdfu_state;
		buf[5] = 0; /* iString not used here */
		usbd_ep0_transfer(usbd_dev, setup_data, buf, sizeof(buf), usbdfu_getstatus_complete);
	return;
	}
	case DFU_GETSTATE:{
		/* Return state with no state transision. */
		usbd_ep0_transfer(usbd_dev, setup_data, &usbdfu_state, 1, NULL);
	return;
	}}

	usbd_ep0_stall(usbd_dev);
}

void __attribute__((weak))
usbdfu_target_usbd_after_init_and_before_first_poll(void) { /* empty */ }

int main(void)
{
	usbd_device *usbd_dev;

	usbdfu_target_init();

	usbd_dev = usbd_init(usbdfu_target_usb_driver(), NULL, &info);

	usbd_register_setup_callback(usbd_dev, usbdfu_setup_callback);

	usbdfu_target_usbd_after_init_and_before_first_poll();

	while (1) {
		usbd_poll(usbd_dev, 0);
	}
}
