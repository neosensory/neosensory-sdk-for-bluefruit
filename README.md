# Neosensory Bluefruit Library

A library for Arduino projects that allows connecting an Adafruit Bluefruit compatible board to connect via BLE and communicate to a Neosensory Buzz. Please note that while the project is licensed under Apache 2.0, usage of the Neosensory API to interface with Neosensory products is still subject to the [Neosensory Developer API License Agreement](https://neosensory.com/legal/dev-terms-service/).

## Installation

Download the latest zip file from the [releases](https://github.com/neosensory/NeosensoryBluefruit/releases) page and add it to Arduino ([instructions](https://www.arduino.cc/en/guide/libraries#toc4)).

## Dependencies

This library depends on Adafruit's Bluefruit library, included in the [Adafruit Board Support Package (BSP) for nRF52 Boards](https://github.com/adafruit/Adafruit_nRF52_Arduino#bsp-installation) (make sure to go through the install instructions thoroughly and update your bootloader) and on [adamvr's base64 library](https://github.com/adamvr/arduino-base64).

## Hardware

This library connects any Neosensory hardware (currently just Buzz) to a microcontroller with an nRF52 BLE chip. At this point, it has only been tested with the [Adafruit Feather nRF52](https://www.adafruit.com/product/3406) but should work with any of [Adafruit's Bluefruit nRF52 boards](https://github.com/adafruit/Adafruit_nRF52_Arduino#arduino-core-for-adafruit-bluefruit-nrf52-boards).

## Documentation

See library documentation at https://neosensory.github.io/NeosensoryBluefruit/.

## Examples

See the [`connect_and_vibrate.ino`](examples/connect_and_vibrate) example.
