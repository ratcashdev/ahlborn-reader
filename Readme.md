# Ahlborn Preludium II pedalboard reader for Arduino

Arduino project to read signals sent by the Ahlborn Preludium II pedalboard and convert them to regular USB Midi signals so that it is usable with SW like GrandOrgue or Hauptwerk.
Requires an Atmega 32u4 board (Leonardo, Beetle etc.)
Requires the USBMidi library.

The midi signals are sent over channel 10.


**Links:**
* https://www.ti.com/lit/ds/symlink/cd4021b-q1.pdf
* http://pdf.datasheetcatalog.com/datasheet/philips/HEF4006BN.pdf


**NOTE:**
Clock signal inversion was broken for the 4006B. Maybe only if the board is fed with +5V only.
To solve this, it was necessary to swap the resistors in front of the BC 238B transistor so that the 5k resistor is between the BASE and the CLOCK signal and the 38k resistor between GND and the base.
