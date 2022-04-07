# DMX-USB interface running on an Atmega328p

This is an Arduino project implementing an USB-DMX interface compatible with [QLC+](https://qlcplus.org/).
It uses a rewritten port of [UDMX](https://github.com/mirdej/udmx) for implementing DMX,
and [VUSB](https://www.obdev.at/products/vusb/index.html) for implementing USB.

You don't need experience with Arduino, C or C++ to build this; I tried to explain every step without requiring prior knowledge in those areas.
However, you do need experience in soldering and building small circuits.

## Split mode

The interface can be used with one or with two Atmegas. Using two Atmegas is called "split mode".
If split mode is disabled, which is the default, the interface will be very choppy.

## Interface variants

There are two variants of the interface, differing in what protocol they use to communicate with the host.
Both can be used with or without split mode.

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

#### Split mode

There is no circuit diagram for split mode. However, most of the circuit stays the same, except for the following:
- Add a second Atmega including a clock.
- Connect the RS485 adapter to the Tx output of the second Atmega instead of the first Atmega, and
  connect the Rx input of the second Atmega to the Tx output of the first Atmega.
  
  In other words, insert the second Atmega between the first Atmega's Tx output and the RS485 adapter.

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
As you probably guessed, `dmx-hid` is the HID-based variant and `dmx-udmx` the UDMX-based one.
You should set the board to "Arduino Uno" (Tools -> Board -> Arduino AVR Boards -> Arduino Uno).
Then, Arduino should be able to compile both variants without problems.

#### Split mode

Import the `dmx-server` sketch as well, like you imported `dmx-hid` and `dmx-udmx`.
This is the sketch running on the second Atmega.

Also, enable `SPLIT_MODE` in `dmxusb-config-common.h`.
See chapter "Configuration and tuning" for details on how to change configuration.

### Burning firmware

I recommend burning the sketches via an ISP programmer. There are plenty of tutorials on how to burn Arduino sketches using ISP.
If you have a regular Arduino board, you can use it as an ISP programmer: see https://www.arduino.cc/en/pmwiki.php?n=Tutorial/ArduinoISP.

Another way is to use the Arduino bootloader.
In this case, you need an USB-to-Serial adapter for burning sketches (once the bootloader is burned).
Again, you can use a regular Arduino board if you have one where the microcontroller can be removed; for example the non-SMD variant of the Uno.
https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoToBreadboard contains information on how to do this, although not the entire tutorial applies here.

#### Split mode

Burn the `dmx-hid` or `dmx-udmx` sketch to the first Atmega (as without split mode), and the `dmx-server` sketch to the second one.
Keep in mind that the `dmx-hid` and `dmx-udmx` sketches change in behaviour when enabling or disabling split mode,
so you have to re-compile and -burn them as well after en- or disabling split mode.

### Configuration and tuning

All configuration is in `libraries/dmxusb-config` in files starting with `dmxusb-config-`.
In general, "enabling" or "setting" a constant means setting its value to 1,
and "disabling" or "unsetting" means setting the value to 0.

If the interface is choppy, try enabling `ATOMIC_UDRE` in `dmxusb-config-dmx.h`.
This makes DMX transmissions much more reliable; at the cost of more errors on the USB side.
This only makes a difference if you don't use split mode.

## Related links

 - [QLC+](https://qlcplus.org/)
 - [VUSB project website](https://www.obdev.at/products/vusb/index.html)
 - [VUSB sources](https://github.com/obdev/v-usb)
 - [UDMX project website](https://anyma.ch/research/udmx/)
 - [UDMX sources](https://github.com/mirdej/udmx)
 - [FX5 project website](http://fx5.de/) (broken; still accessible in the [Wayback Machine](https://web.archive.org/web/20180828195509/https://fx5.de/))
 - [FX5 standalone software](https://github.com/fx5/usbdmx)
