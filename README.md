# Neosensory SDK for Adafruit Bluefruit

An Arduino library that allows BLE communication to a Neosensory Buzz wristband via an Adafruit Bluefruit compatible board. This SDK is centered around the NeosensoryBluefruit class. 

## Installation

Download the latest zip file from the [releases page](https://github.com/neosensory/NeosensoryBluefruit/releases) and add it to Arduino following these [instructions for adding zip file libraries to Arduino](https://www.arduino.cc/en/guide/libraries#toc4).

## Dependencies

This library depends on Adafruit's Bluefruit library, included in the [Adafruit Board Support Package (BSP) for nRF52 Boards](https://github.com/adafruit/Adafruit_nRF52_Arduino#bsp-installation) (make sure to go through the install instructions thoroughly and update your bootloader) and on [adamvr's base64 library](https://github.com/adamvr/arduino-base64).

## Hardware

This library connects any Neosensory hardware (currently just Buzz) to a microcontroller with an nRF52 BLE chip. At this point, it has only been tested with the [Adafruit Feather nRF52](https://www.adafruit.com/product/3406) and [nRF52840 Express](https://www.adafruit.com/product/4062) but should work with any of [Adafruit's Bluefruit nRF52 boards](https://github.com/adafruit/Adafruit_nRF52_Arduino#arduino-core-for-adafruit-bluefruit-nrf52-boards).

## Documentation

See library documentation at https://neosensory.github.io/neosensory-sdk-for-bluefruit/. See GitHub repo at https://github.com/neosensory/neosensory-sdk-for-bluefruit.

## Examples

See the [`connect_and_vibrate.ino`](https://github.com/neosensory/neosensory-sdk-for-bluefruit/blob/master/examples/connect_and_vibrate/connect_and_vibrate.ino) example.

## Pairing

Whether for the `connect_and_vibrate.ino` example or for your own project, you'll need to put Buzz into pairing mode the first time you connect to it. To do this, turn on your Buzz wristband and press and hold the plus and minus buttons on top of your Buzz. Buzz will show three blue LEDs and then a random pattern of LEDs (which is included in the advertising packet information in case you need to differentiate from several different Buzzes in pairing mode, but for most situations can be ignored). 

## License

Please note that while this Neosensory SDK has an Apache 2.0 license, 
usage of the Neosensory API to interface with Neosensory products is 
still  subject to the Neosensory developer terms of service located at:
https://neosensory.com/legal/dev-terms-service.

See [LICENSE](https://github.com/neosensory/neosensory-sdk-for-bluefruit/blob/master/LICENSE).

## Author

Originally created by Mike Perrotta for Neosensory, Inc.
