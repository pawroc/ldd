obj-m := pcd.o

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules

clean:
	make -C $(HOST_KERN_DIR) M=$(PWD) clean

help:
	make -C $(HOST_KERN_DIR) M=$(PWD) help