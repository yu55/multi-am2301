MOD=multi-am2301

obj-m := $(MOD).o

$(MOD)-objs := stats.o decoder.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
