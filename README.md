# UDMX-based DMX-USB interface running on an Atmega328p

Work-in-progress.

This is an Arduino project aiming to implement an USB-DMX interface compatible with [QLC+](https://qlcplus.org/) via the (preinstalled) UDMX plugin,
by porting the [UDMX](https://anyma.ch/research/udmx/) USB-DMX interface to Atmega328p.

On Windows, the [libusbK driver](https://www.illutzminator.de/udmxdriver.html) needs to be installed for this to work. (If you don't want to install a driver, use the [HID-based interface](https://github.com/Haspamelodica/dmx-usb-atmega328p-hid/), which works without a driver.)

HID-based interface: https://github.com/Haspamelodica/dmx-usb-atmega328p-hid/

Related link: [Original UDMX sources](https://github.com/mirdej/udmx)

# HID-based DMX-USB interface running on an Atmega328p

Work-in-progress.

This is an Arduino project aiming to implement an USB-DMX interface compatible with [QLC+](https://qlcplus.org/) via the (preinstalled) HID plugin,
by using the same protocol as the [FX5](http://fx5.de/) and [Digital Englightment](http://www.digital-enlightenment.de/usbdmx.htm) USB-DMX interfaces.

UDMX-based version: https://github.com/Haspamelodica/dmx-usb-atmega328p-udmx/

Related link: [Host-side standalone software](https://github.com/fx5/usbdmx)
