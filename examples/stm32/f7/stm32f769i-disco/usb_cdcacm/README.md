# README

This example implements a USB CDC-ACM device (aka Virtual Serial Port)
to demonstrate the use of the USB device stack.

## Configuration
at compile time, `#define SHOW_HIGH_SPEED_DEMO` to enumerate as
HIGH speed device.

## Board connections

| Port  | Function       | Description                               |
| ----- | -------------- | ------------------------------------------ |
| `CN5` | `(USB_OTG_HS)` | USB acting as device, connect to computer |
