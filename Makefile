obj-m += steno.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod steno.ko

remove:
	sudo rmmod steno

reload:
	-sudo rmmod steno
	sudo insmod steno.ko

test:
	sudo dmesg -w

.PHONY: all clean install remove reload test