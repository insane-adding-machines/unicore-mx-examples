#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/flash.h>
#include <unicore-mx/usbd/usbd.h>
#include "usbdfu-target.h"

#define APP_ADDRESS	0x08002000

/* This string is used by ST Microelectronics' DfuSe utility. */
const struct usb_string_descriptor string_st_flash_detail = {
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

void usbdfu_target_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);

	if (!gpio_get(GPIOA, GPIO10)) {
		/* Boot the application if it's valid. */
		if ((*(volatile uint32_t *)APP_ADDRESS & 0x2FFE0000) == 0x20000000) {
			/* Set vector table base address. */
			SCB_VTOR = APP_ADDRESS & 0xFFFF;
			/* Initialise master stack pointer. */
			asm volatile("msr msp, %0"::"g"
				     (*(volatile uint32_t *)APP_ADDRESS));
			/* Jump to application. */
			(*(void (**)())(APP_ADDRESS + 4))();
		}
	}

	rcc_clock_setup_in_hsi_out_48mhz();

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_AFIO);

	AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON;
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, 0, GPIO15);

	rcc_periph_clock_enable(RCC_OTGFS);
}

const usbd_backend *usbdfu_target_usb_driver(void)
{
	return USBD_STM32_OTG_FS;
}

void usbdfu_target_usbd_after_init_and_before_first_poll(void)
{
	gpio_set(GPIOA, GPIO15);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO15);
}

/* flash operation */

void usbdfu_target_flash_lock(void)
{
	flash_lock();
}

void usbdfu_target_flash_unlock(void)
{
	flash_unlock();
}

void usbdfu_target_flash_erase_page(uint32_t page)
{
	flash_erase_page(page);
}

void usbdfu_target_flash_write(uint32_t baseaddr, uint8_t *buf, uint16_t len)
{
	uint32_t i;

	for (i = 0; i < len; i += 2) {
		uint16_t *dat = (uint16_t *)(buf + i);
		flash_program_half_word(baseaddr + i, *dat);
	}
}
