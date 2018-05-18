#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/stm32/crs.h>
#include <unicore-mx/stm32/flash.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/syscfg.h>

#include "usbdfu-target.h"

#define APP_ADDRESS		0x08002000

/* This string is used by ST Microelectronics' DfuSe utility. */
const struct usb_string_descriptor string_st_flash_detail = {
	.bLength = USB_DT_STRING_SIZE(46),
	.bDescriptorType = USB_DT_STRING,
	/* @Internal Flash   /0x08000000/8*001Ka,24*001Kg */
	.wData = {
		0x0040, 0x0049, 0x006e, 0x0074, 0x0065, 0x0072, 0x006e, 0x0061,
		0x006c, 0x0020, 0x0046, 0x006c, 0x0061, 0x0073, 0x0068, 0x0020,
		0x0020, 0x0020, 0x002f, 0x0030, 0x0078, 0x0030, 0x0038, 0x0030,
		0x0030, 0x0030, 0x0030, 0x0030, 0x0030, 0x002f, 0x0038, 0x002a,
		0x0030, 0x0030, 0x0031, 0x004b, 0x0061, 0x002c, 0x0032, 0x0034,
		0x002a, 0x0030, 0x0030, 0x0031, 0x004b, 0x0067
	}
};

void usbdfu_target_init(void)
{
	/* Boot the application if it's valid. */
	if ((*(volatile uint32_t *)APP_ADDRESS & 0x2FFE0000) == 0x20000000) {
		/* Remap shadow memory to SRAM */
		SYSCFG_CFGR1 &= ~(SYSCFG_CFGR1_MEM_MODE);
		SYSCFG_CFGR1 |= SYSCFG_CFGR1_MEM_MODE_SRAM;

		/* Initialise master stack pointer */
		asm volatile("msr msp, %0"::"g"
			     (*(volatile uint32_t *)APP_ADDRESS));

		/* Jump to application */
		(*(void (**)())(APP_ADDRESS + 4))();
	}

	/* start HSI48 */
	rcc_clock_setup_in_hsi48_out_48mhz();

	/* use usb SOF as correction source for HSI48 */
	crs_autotrim_usb_enable();

	/* use HSI48 for USB */
	rcc_set_usbclk_source(RCC_HSI48);

	/* usb HSI48 for system clock */
	rcc_set_sysclk_source(RCC_HSI48);

	rcc_periph_clock_enable(RCC_SYSCFG_COMP);

	/* PA11_PA12_RMP */
	SYSCFG_CFGR1 |= 1 << 4;
}

const usbd_backend *usbdfu_target_usb_driver(void)
{
	return USBD_STM32_FSDEV;
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
