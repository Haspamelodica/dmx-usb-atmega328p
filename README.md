# DMX-USB interface running on an Atmega328p

Work-in-progress.

This is an Arduino project aiming to implement an USB-DMX interface compatible with [QLC+](https://qlcplus.org/).
There are two variants of the interface. They differ in what protocol they use to communicate with the host. One is UDMX-based and one is HID-based.
Both variants use a rewritten port of UDMX for implementing the DMX protocol.

## Interface variants

### UDMX-based interface

This variant works with the the (preinstalled) UDMX plugin of QLC+, by porting the [UDMX](https://anyma.ch/research/udmx/) USB-DMX interface to Atmega328p.

On Windows, the [libusbK driver](https://www.illutzminator.de/udmxdriver.html) needs to be installed for this to work.
(If you don't want to install a driver, use the HID-based interface, which works without a driver.)

### HID-based interface

Works with the (preinstalled) HID plugin of QLC+, by using the same protocol
as the [FX5](http://fx5.de/) and [Digital Englightment](http://www.digital-enlightenment.de/usbdmx.htm) USB-DMX interfaces.

UDMX-based version: https://github.com/Haspamelodica/dmx-usb-atmega328p-udmx/

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
 - [Original UDMX sources](https://github.com/mirdej/udmx)
 - [Host-side standalone software for HID variant](https://github.com/fx5/usbdmx)
