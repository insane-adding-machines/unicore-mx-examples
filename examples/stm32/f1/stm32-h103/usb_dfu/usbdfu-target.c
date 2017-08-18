#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/stm32/flash.h>
#include "usbdfu-target.h"

#define APP_ADDRESS	0x08002000

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

	rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);
	gpio_set(GPIOC, GPIO11);
}

const usbd_backend *usbdfu_target_usb_driver(void)
{
	return USBD_STM32_FSDEV;
}

void usbdfu_target_usbd_after_init_and_before_first_poll(void)
{
	gpio_clear(GPIOC, GPIO11);
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
