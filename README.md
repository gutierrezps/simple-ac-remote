# Simple AC Remote Control

This is a stripped-down programmable AC remote control made with an Arduino Nano board. Software is based on [Shirriff's IRremote library](https://github.com/z3t0/Arduino-IRremote).

The remote has two buttons and three LEDs. One button turns off the AC unit, and the other sets the cooling level (1, 2 or 3), shown by the LEDs.

IR code programming is still in progress. Next step is to detect the protocol and then store the data bytes in EEPROM memory.

### Arduino Pins Assignment

* 2 - IR sensor (VS1838)
* 3 - IR LED (driven by a transistor)
* 4 - LED 1
* 5 - LED 2
* 6 - LED 3
* 11 - Off button (`INPUT_PULLUP`)
* 12 - Level buton (`INPUT_PULLUP`)


# Copyright
Copyright 2017 Gutierrez PS