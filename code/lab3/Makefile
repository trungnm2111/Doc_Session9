obj-m += trungled_drv.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) MO=$(PWD)/output modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/output clean