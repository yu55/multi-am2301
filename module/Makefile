MOD=multi-am2301

obj-m := $(MOD).o

$(MOD)-objs := decoder.o stats.o

all:
	ARCH=arm CROSS_COMPILE=${CCPREFIX} $(MAKE) -C /lib/modules/4.4.26+/build M=$(PWD) modules

clean:
	ARCH=arm CROSS_COMPILE=${CCPREFIX} $(MAKE) -C /lib/modules/4.4.26+/build M=$(PWD) clean
