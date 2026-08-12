obj-m += netfilter_firewall.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod netfilter_firewall.ko

remove:
	sudo rmmod netfilter_firewall

log:
	sudo dmesg | tail -n 30

reload: remove all install
	@echo "Module reloaded"

.PHONY: all clean install remove log reload
