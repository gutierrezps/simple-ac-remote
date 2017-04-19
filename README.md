# Simple AC Remote Control

This is a stripped-down programmable AC remote control made with Arduino Pro Mini. Software is based on [Shirriff's IRremote library](https://github.com/z3t0/Arduino-IRremote).

The remote has two buttons and three LEDs. One button turns off the AC unit, and the other sets the cooling level (1, 2 or 3), shown by the LEDs.

IR code programming is still in progress. Next step is to detect the protocol and then store the data bytes in EEPROM memory.

### Arduino Pins Assignment

* 2 - IR sensor (VS1838)
* 3 - IR LED (NPN/N-MOSFET transistor driven)
* 4 - LED 1
* 5 - LED 2
* 6 - LED 3
* 7 - Blink LED (on send)
* 11 - Off button (`INPUT_PULLUP`)
* 12 - Level buton (`INPUT_PULLUP`)


# Resources and other projects

* [HVAC IR Control](https://github.com/r45635/HVAC-IR-Control)
* [Cooking hacks IR Remote shield](https://www.cooking-hacks.com/documentation/tutorials/control-hvac-infrared-devices-from-the-internet-with-ir-remote/)
* [VS1838B IR receiver module](http://arduino-kit.ru/userfiles/image/VS1838B_INFRARED_RECEIVER_MODULE.pdf)
* [VS1838B datasheet (in english)](https://www.optimusdigital.ro/index.php?controller=attachment&id_attachment=4&usg=afqjcnh7fun0zdtnznv0ufaeth9sjdsgtg&sig2=t1sm5ow3vxetkff9hq2tsw) 


# Copyright
Copyright 2017 Gutierrez PS