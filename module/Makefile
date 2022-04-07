MOD=multi-am2301

obj-m := $(MOD).o

$(MOD)-objs := stats.o decoder.o

all:
	ARCH=arm CROSS_COMPILE=${CCPREFIX} $(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	ARCH=arm CROSS_COMPILE=${CCPREFIX} $(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	cp $(MOD).ko /lib/modules/$(shell uname -r)/kernel/drivers/
	depmod
	ls -Al /lib/modules/$(shell uname -r)/kernel/drivers/$(MOD).ko
