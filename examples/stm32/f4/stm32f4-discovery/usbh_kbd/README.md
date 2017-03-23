# README

This example implements a USB Host that read from keyboard (boot)
to demonstrate the use of the USB host stack.

Compile time option is provided to select between OTG_FS and OTG_HS

## Board connections

| Port   | Function       | Description                               |
| -------| -------------- | ----------------------------------------- |
| `CN14` | `(USB_OTG_FS)` | USB acting as host, connect to keyboard   |
| `CN12` | `(USB_OTG_HS)` | USB acting as host, connect to keyboard   |
| `PC6`  | `(USART6 TX)`  | USART Log output                          |
