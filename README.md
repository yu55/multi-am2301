# multi-am2301
This repository contains Linux kernel module reading temperature and relative humidity data from multiple AM2301/AM2023 (DHT11/DHT21/DHT22) sensors simultaneously connected to Raspberry Pi via GPIO pins.

This kernel module is an evolution of a module presented on [Blackwire Embedded blog](http://www.blackwire.ro/index.php/site-map/articles/79-embedded/raspberrypy/76-am2301-dht21-temperature-rh-sensor-with-raspberry-pi-kernel-module). Compilation issues against `proc_fs` were fixed and multiple sensors support added. It works stable with kernels: `Linux rpi 5.15.32-v7l+ #1538 SMP Thu Mar 31 19:39:41 BST 2022 armv7l GNU/Linux`. It should work with other kernel versions as well unless some major API changes were done in the kernel.

![multi-am2301](/multi-am2301.png?raw=true "View of data from AM2301 sensors connected to Raspberry Pi") ![outside_t](/outside_t.png?raw=true "Plot of data collected by multi-am2301")


Latest measurement data from connected sensors is available under `/proc/multi-am2301` and can be displayed anytime using e.g. following command: `cat /proc/multi-am2301`. For each connected sensor it contains parameters like:
* `temp_curr` - value of last temperature measurement in Celsius degrees
* `temp_1m` - average value of temperature for last 1 minute
* `RH_curr` - value of last relative humidity measurement
* `RH_1m` - average value of relative humidity for last 1 minute
* `date` - formatted measurement date and time
* `timestamp` - measurement timestamp (millis from Epoch)
* `QUAL` - how many sensor read attempts were there and how many of them were successfull. Note: idling Raspberry Pi has `QUAL` more than 90% for each correctly connected sensor, however when CPU is fully loaded or lots of interrputs occure (e.g. when reading lots of data from serial communication etc.) `QUAL` will drop down.

## Repository layout
Kernel module sits inside `module` directory. Example commandline script which logs temperature and humidity into sqlite database sits inside `scripts` directory. This script can be invoked by system cron e.g. every 3 minutes.

## Installation
* clone this repository
* `cd module`
* in source file `decoder.c` find `static int _pins[] = { 5, 6, 12};` array initialization and correct GPIO logical pin numbers according to your needs (don't use board physical pin numbers!)
* please check `/lib/modules/VERSION/build` directory in `Makefile` against your OS and correct if needed. Note: you need kernel headers present in your system to be able to build kernel modules. You may want to check project [rpi-source](https://github.com/notro/rpi-source)
* execute `make`
* `sudo cp multi-am2301.ko /lib/modules/{VERSION}/kernel/drivers/`
* `sudo depmod`
* to load kernel module: `sudo modprobe multi-am2301`
* to unload kernel module: `sudo modprobe -r multi-am2301`

Additionally to load this kernel module on Raspberry Pi startup add `multi-am2301` to `/etc/modules` configuration file.

Have fun :)

