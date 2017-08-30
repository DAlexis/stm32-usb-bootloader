# USB bootloader for STM32F1 microcontrollers

This repository contains bootloader for `STM32F1xx` microcontrollers. It was tested only for `STM32F103RET6`, but you may easily adopt it to other MCUs from `F1` series supporting STM32 HAL library.

## What do you need?

To use this bootloader you should have:

- SD or MicroSD card connected to your MCU via 4-wire SDIO interface (do not forget 47kOhm resistors!). MicroSD card should works properly! File system should be Fat32.

- Optionally, USB 2.0 port (type B, slave device), connected to your MCU. Do not forget 1.5kOhm resistor between D+ and 3.3V, otherwise your device will not be discovered by a computer.

## How to use bootloader

You can put file `flash.bin` to SD-card with your flash image. There is no restrictions to image except it's size: it should be a bit smaller then MCU flash size to leave some place for bootloader. No image modifications (such as moving IRQ table or other) needed! You have two possibilities to put this file:

- Enject microSD card and put `flash.bin` directly

- Connect you MCU over USB to computer and run device. No drivers needed, your chip will run in cardreader mode. Copy `flash.bin` to MicroSD over this connection.

Than disconnect your device from PC (if connected), reboot it and after tens seconds your image will be flashed to MCU and run. Bootloader checks hash sum on start, so there is no reflashing every boot.

Note: when your MCU starts with bootloader, it takes control for 1 second to wait connection from PC and than boot your program. In this time all pins are pulled to zero if possible. You may change this behaviour.


## Contacts

You are free to communicate with me by email: `AlexeyABulatov@yandex.ru`
