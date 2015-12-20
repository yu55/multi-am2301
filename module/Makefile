MOD=multi-am2301

obj-m := $(MOD).o

all:
	ARCH=arm CROSS_COMPILE=${CCPREFIX} $(MAKE) -C /lib/modules/3.12.22+/build M=$(PWD) modules

clean:
	ARCH=arm CROSS_COMPILE=${CCPREFIX} $(MAKE) -C /lib/modules/3.12.22+/build M=$(PWD) clean
