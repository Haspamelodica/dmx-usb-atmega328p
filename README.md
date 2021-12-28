# DMX-USB interface running on an Atmega328p

Work-in-progress.

This is an Arduino project aiming to implement an USB-DMX interface compatible with [QLC+](https://qlcplus.org/).
It uses a rewritten port of [UDMX](https://github.com/mirdej/udmx) for implementing DMX,
and [VUSB](https://www.obdev.at/products/vusb/index.html) for implementing USB.

## Interface variants

There are two variants of the interface, differing in what protocol they use to communicate with the host.

### UDMX-based interface

This variant works with the the (preinstalled) UDMX plugin of QLC+,
by porting the [UDMX USB-DMX interface]((https://anyma.ch/research/udmx/)) to Atmega328p.

On Windows, the [libusbK driver](https://www.illutzminator.de/udmxdriver.html) needs to be installed for this to work.
This repository contains a copy of the driver software: `uDMX_Driver_libUSBK.zip`.
(If you don't want to install a driver, use the HID-based interface, which works without a driver.)

### HID-based interface

Works with the (preinstalled) HID plugin of QLC+, by using the same protocol
as the [FX5](http://fx5.de/) and [Digital Englightment](http://www.digital-enlightenment.de/usbdmx.htm) USB-DMX interfaces.

The USB and HID communication part of this project is based on the `hid-data` example of VUSB.

## Building the interface

### The circuit

To build the interface, you need to build a small circuit.
The circuit is in `circuit/circuit.fzz` and can be opened with [Fritzing](https://fritzing.org/).
If you don't want to install Fritzing, screenshots are available as well.

### Compiling firmware

Both interface variants come in the form of Arduino sketches.
This means you can (and should) use the [Arduino software](https://www.arduino.cc/en/software) to compile the interface.
For this, you have to import the sketches into Arduino.
- The easiest way is to copy the `dmx-hid`, `dmx-udmx`, and `libraries` folders into your sketchbook folder.
  (There will probably already be a folder called `libraries` in the sketchbook. In this case, merge both folders.)
- Another way is to temporarily change Arduino's sketchbook path to your local copy of this repository.
  Don't forget where your main sketchbook folder is if you have other projects and want to change back later.
- The ugliest way is to create symlinks. You should only do this if you know what symlinks are.

After importing, the Arduino software will see `dmx-hid` and `dmx-udmx` as regular sketches.
(As you probably guessed, `dmx-hid` is the HID-based variant and `dmx-udmx` the UDMX-based one.)
You should set the board to "Arduino Uno" (Tools -> Board -> Arduino AVR Boards -> Arduino Uno).
Then, Arduino should be able to compile both variants without problems.

### Uploading firmware

TODO

## Related links
 - [QLC+](https://qlcplus.org/)
 - [VUSB project website](https://www.obdev.at/products/vusb/index.html)
 - [VUSB sources](https://github.com/obdev/v-usb)
 - [UDMX project website](https://anyma.ch/research/udmx/)
 - [UDMX sources](https://github.com/mirdej/udmx)
 - [FX5 project website](http://fx5.de/) (broken; still accessible in the [Wayback Machine](https://web.archive.org/web/20180828195509/https://fx5.de/))
 - [FX5 standalone software](https://github.com/fx5/usbdmx)
