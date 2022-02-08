obj-m   := driver.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all:
        $(MAKE) -C $(KERNELDIR) M=$(PWD)
clean:
        make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
