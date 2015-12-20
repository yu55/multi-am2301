# multi-am2301
Linux kernel module that supports multiple AM2301 (DHT21) sensors simultaneously connected to Raspberry Pi.

This kernel module is an evolution of a module presented on [Blackwire Embedded blog](http://www.blackwire.ro/index.php/site-map/articles/79-embedded/raspberrypy/76-am2301-dht21-temperature-rh-sensor-with-raspberry-pi-kernel-module). Compilation issues against `proc_fs` were fixed and multiple sensors support added. It works stable with kernel: `Linux raspberrypi 3.12.22+ #691 PREEMPT Wed Jun 18 18:29:58 BST 2014 armv6l GNU/Linux`.

## Repository layout
Kernel module sits inside `module` directory. Example commandline script which logs temperature and humidity into sqlite database sits inside `scripts` directory. This script can be invoked by e.g. system cron every 3 minutes.

