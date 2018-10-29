# README

This example shows how to use DMA2 controller to get data from ADC1
channels 0 and 1. ADC1 data measurements are printed periodically on
USART2 console (Tx only) 115200@8n1.

Pin layout:

  * PA0: ADC1 channel 0.
  * PA1: ADC1 channel 1.
  * PA2: USART2 Tx.

Example output:

    Starting adc-dma example!
    ADC1: ch0=291, ch1=509
    ADC1: ch0=297, ch1=512
    ADC1: ch0=285, ch1=515
    ADC1: ch0=290, ch1=518
    [...]
