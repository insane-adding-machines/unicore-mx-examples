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
#include <unicore-mx/usbd/misc/string.h>
#include "usbdfu-target.h"

/* Commands sent with wBlockNum == 0 as per ST implementation. */
/* FIXME: as per above not, this code is still infected with ST implementation */
#define CMD_SETADDR	0x21
#define CMD_ERASE	0x41

/* We need a special large control buffer for this device: */
uint8_t usbd_control_buffer[1024];

static enum dfu_state usbdfu_state = STATE_DFU_IDLE;

static struct {
	uint8_t buf[sizeof(usbd_control_buffer)];
	uint16_t len;
	uint32_t addr;
	uint16_t blocknum;
} prog;

const struct usb_dfu_descriptor dfu_function = {
	.bLength = sizeof(struct usb_dfu_descriptor),
	.bDescriptorType = DFU_FUNCTIONAL,
	.bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
	.wDetachTimeout = 255,
	.wTransferSize = 1024,
	.bcdDFUVersion = 0x011A,
};

const struct usb_interface_descriptor iface = {
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

	.extra = &dfu_function,
	.extra_len = sizeof(dfu_function),
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
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,

	.interface = ifaces,
}};

/* This string is used by ST Microelectronics' DfuSe utility. */
/* FIXME: this is ST Specific */
#define ST_FLASH_DETAIL "@Internal Flash   /0x08000000/8*001Ka,56*001Kg"

const uint8_t *usb_string_english[] = {
	(uint8_t *) "Black Sphere Technologies",
	(uint8_t *) "DFU Demo",
	(uint8_t *) "DEMO",
	(uint8_t *) ST_FLASH_DETAIL
};

/*
 * Black = काला
 * Sphere = गोला
 * Technologies = प्रौद्योगिकी
 * DFU = डीएफयू
 * Demo, DEMO = नमूना
 */
const uint8_t *usb_string_hindi[] = {
	(uint8_t *) "काला गोला प्रौद्योगिकी",
	(uint8_t *) "डीएफयू नमूना",
	(uint8_t *) "नमूना",
	(uint8_t *) ST_FLASH_DETAIL
};

const uint16_t supported_lang[] = {
	USB_LANGID_ENGLISH_UNITED_STATES,
	USB_LANGID_HINDI
};

static int usb_strings(usbd_device *usbd_dev, struct usbd_get_string_arg *arg)
{
	(void) usbd_dev;

	const uint8_t **strings;

	/* supported languages */
	if (!arg->index) {
		uint16_t len = arg->len;
		if (len > 2) {
			len = 2;
		}
		memcpy(arg->buf, supported_lang, len);
		return len;
	}

	/* we only have 4 strings */
	if (arg->index > 4) {
		return -1;
	}

	/* language */
	switch(arg->lang_id) {
	case USB_LANGID_ENGLISH_UNITED_STATES:
		strings = usb_string_english;
		break;
	case USB_LANGID_HINDI:
		strings = usb_string_hindi;
		break;
	default:
		return -1;
	}

	return usbd_utf8_to_utf16(strings[arg->index - 1], arg->buf, arg->len);
}

const struct usb_device_descriptor dev = {
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
	.bNumConfigurations = 1,

	.config = config,
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

static void usbdfu_getstatus_complete(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	(void)usbd_dev;
	(void)arg;

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
				       dfu_function.wTransferSize);
			usbdfu_target_flash_write(baseaddr, prog.buf, prog.len);
		}
		usbdfu_target_flash_lock();

		/* Jump straight to dfuDNLOAD-IDLE, skipping dfuDNLOAD-SYNC. */
		usbdfu_state = STATE_DFU_DNLOAD_IDLE;
		return;
	case STATE_DFU_MANIFEST:
		/* USB device must detach, we just reset... */
		scb_reset_system();
		return; /* Will never return. */
	default:
		return;
	}
}

static enum usbd_control_result
usbdfu_control_request(usbd_device *usbd_dev, struct usbd_control_arg *arg)
{
	(void)usbd_dev;
	const uint8_t bmReqMask = USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT;
	const uint8_t bmReqVal = USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE;

	if ((arg->setup.bmRequestType & bmReqMask) != bmReqVal) {
		return USBD_REQ_NEXT;
	}

	switch (arg->setup.bRequest) {
	case DFU_DNLOAD:
		if (!arg->len) {
			usbdfu_state = STATE_DFU_MANIFEST_SYNC;
			return USBD_REQ_HANDLED;
		} else {
			/* Copy download data for use on GET_STATUS. */
			prog.blocknum = arg->setup.wValue;
			prog.len = arg->len;
			memcpy(prog.buf, arg->buf, arg->len);
			usbdfu_state = STATE_DFU_DNLOAD_SYNC;
			return USBD_REQ_HANDLED;
		}
	case DFU_CLRSTATUS:
		/* Clear error and return to dfuIDLE. */
		if (usbdfu_state == STATE_DFU_ERROR) {
			usbdfu_state = STATE_DFU_IDLE;
		}
		return USBD_REQ_HANDLED;
	case DFU_ABORT:
		/* Abort returns to dfuIDLE state. */
		usbdfu_state = STATE_DFU_IDLE;
		return USBD_REQ_HANDLED;
	case DFU_UPLOAD:
		/* Upload not supported for now. */
		return USBD_REQ_STALL;
	case DFU_GETSTATUS: {
		uint32_t bwPollTimeout = 0; /* 24-bit integer in DFU class spec */
		arg->buf[0] = usbdfu_getstatus(&bwPollTimeout);
		arg->buf[1] = bwPollTimeout & 0xFF;
		arg->buf[2] = (bwPollTimeout >> 8) & 0xFF;
		arg->buf[3] = (bwPollTimeout >> 16) & 0xFF;
		arg->buf[4] = usbdfu_state;
		arg->buf[5] = 0; /* iString not used here */
		arg->len = 6;
		arg->complete = usbdfu_getstatus_complete;
		return USBD_REQ_HANDLED;
		}
	case DFU_GETSTATE:
		/* Return state with no state transision. */
		arg->buf[0] = usbdfu_state;
		arg->len = 1;
		return USBD_REQ_HANDLED;
	}

	return USBD_REQ_STALL;
}

void __attribute__((weak))
usbdfu_target_usbd_after_init_and_before_first_poll(void) { /* empty */ }

int main(void)
{
	usbd_device *usbd_dev;

	usbdfu_target_init();

	usbd_dev = usbd_init(usbdfu_target_usb_driver(), &dev,
		usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_control_callback(usbd_dev, usbdfu_control_request);
	usbd_register_get_string_callback(usbd_dev, usb_strings);

	usbdfu_target_usbd_after_init_and_before_first_poll();

	while (1) {
		usbd_poll(usbd_dev);
	}
}
