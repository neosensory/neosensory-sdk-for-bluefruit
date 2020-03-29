# Neosensory Bluefruit Library

A library for Arduino projects that allows connecting an Adafruit Bluefruit compatible board to connect via BLE and communicate to a Neosensory Buzz. 

## Installation

Download the latest zip file from the [releases](https://github.com/neosensory/NeosensoryBluefruit/releases) page and add it to Arduino ([instructions](https://www.arduino.cc/en/guide/libraries#toc4)).

## Dependencies

This library depends on Adafruit's Bluefruit library, included in the [Adafruit Board Support Package (BSP) for nRF52 Boards](https://github.com/adafruit/Adafruit_nRF52_Arduino) (make sure to go through the install instructions thoroughly and update your bootloader) and on [adamvr's base64 library](https://github.com/adamvr/arduino-base64).

## Hardware

This library connects any Neosensory hardware (currently just Buzz) to a microcontroller with an nRF52 BLE chip. At this point, it has only been tested with the [Adafruit Feather nRF52](https://www.adafruit.com/product/3406) but should work with any of [Adafruit's Bluefruit nRF52 boards](https://github.com/adafruit/Adafruit_nRF52_Arduino#arduino-core-for-adafruit-bluefruit-nrf52-boards).

## Documentation

See library documentation at https://neosensory.github.io/NeosensoryBluefruit/. _(Documentation will be published when this repo becomes public. For now, see documentation by downloading `gh-pages` branch and opening `index.html`.)_

## Examples

See the [`connect_and_vibrate.ino`](examples/connect_and_vibrate) example. For use with a Neosensory Buzz, you will need to supply your device ID, which can be obtained by connecting to your Buzz over USB over a serial terminal application (baud: 9600) and running the command `device info`. After successfully uploading the example code with the correct device ID, into pairing mode by holding the (+) and (-) buttons simultaneously until the blue LEDs flash, which will then enable the Bluefruit device to connect and begin controlling the Buzz.
