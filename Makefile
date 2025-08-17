obj-m += teclas_vb.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod teclas_vb.ko

remove:
	sudo rmmod teclas_vb

reload:
	-sudo rmmod teclas_vb
	sudo insmod teclas_vb.ko

test:
	dmesg | tail -20

.PHONY: all clean install remove reload test