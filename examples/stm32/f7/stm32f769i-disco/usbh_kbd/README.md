# README

This example implements a USB Host that read from keyboard (boot)
to demonstrate the use of the USB host stack.

## Board connections

| Port          | Function       | Description                               |
| ------------- | -------------- | ----------------------------------------- |
| `CN15`        | `(USB_OTG_HS)` | USB acting as host, connect to keyboard   |
| `PC6`         | `(USART6 TX)`  | USART Log output                          |
