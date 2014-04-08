BASE := test
obj-m := $(BASE).o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf modules.order Module* .tmp* $(BASE).ko* $(BASE).o* $(BASE).mod* .$(BASE)*
