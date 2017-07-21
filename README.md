# Simple AC Remote Control

This is a stripped-down programmable AC remote control made with Arduino Pro Mini. Software is based on [Shirriff's IRremote library](https://github.com/z3t0/Arduino-IRremote).

The remote has two buttons and three LEDs. One button turns off the AC unit, and the other sets the cooling level (1, 2 or 3), shown by the LEDs.


### Arduino Pins Assignment

* 2 - IR sensor (VS1838 output)
* 3 - IR LED (NPN/N-MOSFET transistor driven)
* 4 - LED 1
* 5 - LED 2
* 6 - LED 3
* 7 - Blink LED (on send and for other info)
* 10 - Level buton (`INPUT_PULLUP`)
* 11 - Off button (`INPUT_PULLUP`)


## How it's done

Instead of decoding specific remote control data, like number of bits, function of each bit, checksum etc, this project aims to reproduce whatever command is presented with a timing decode/encode approach, i.e. raw data is transformed to bits by association with a generic protocol, defined by some timing paratemers.

Once the decode process is done, the data bits are stored on Arduino's EEPROM memory, and then loaded on next system boot.


## File structure

``IRData.hpp`` defines a class that holds an IR data packet, consisting of a protocol reference and the decoded data bits.

``IRProtocols.hpp`` defines an IR protocol class with its timings and arbitrary ID number and name; and also defines a storage class for all protocols used in this project.

``IRDecoder.hpp`` and ``IRSender.hpp`` defines functions for decoding and encoding of IR data. The decode process compares the raw data provided by IRremote library with the available protocols.

``IRRawAnalyzer.hpp`` holds a function that analyzes the raw IR data, basically counting and printing the occurence of each width found, useful to debug and identify new protocols.


## Notes

The following changes were made on IRremote library:

On file ``IRremoteInt.h``:
* ``RAWBUF`` to ``300``, in order to capture bigger packets.
* ``_GAP`` to ``10000``, in order to capture shorter headers and repeated data.
* struct ``irparams_t``, and also on file ``IRremote.h`` class ``decode_results``, change ``rawlen`` type to ``uint16_t``
* ``MARK_EXCESS`` to 0
* ``TOLERANCE`` to 10


## Resources and other projects

* [HVAC IR Control](https://github.com/r45635/HVAC-IR-Control)
* [Cooking hacks IR Remote shield](https://www.cooking-hacks.com/documentation/tutorials/control-hvac-infrared-devices-from-the-internet-with-ir-remote/)
* [VS1838B IR receiver module](http://arduino-kit.ru/userfiles/image/VS1838B_INFRARED_RECEIVER_MODULE.pdf)
* [VS1838B datasheet](https://www.optimusdigital.ro/index.php?controller=attachment&id_attachment=4&usg=afqjcnh7fun0zdtnznv0ufaeth9sjdsgtg&sig2=t1sm5ow3vxetkff9hq2tsw)
* [Random word generator](http://watchout4snakes.com/wo4snakes/Random/RandomWordPlus) : used as protocol name generator (uncommon nouns mode)


## Copyright
Copyright 2017 Gutierrez PS
