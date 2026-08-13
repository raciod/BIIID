obj-m += netfilter_firewall.o

# Tell Kbuild to look in the shared directory for fw_rule.h
ccflags-y := -I$(src)/../shared

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
