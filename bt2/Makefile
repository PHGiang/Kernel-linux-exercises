obj-m = chardev.o
EXTRA_CFLAGS = -Wall
KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd` modules
clean:
	make -C $(KDIR) M=`pwd` clean