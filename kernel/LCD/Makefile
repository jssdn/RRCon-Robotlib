#obj-m += kern1.o
#obj-m += kern3.o
#obj-m += chrdev.o
obj-m += xilinx_lcd.o

all:
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make -C /home/dilbert/xenomai/linux-2.6-denx/ M=$(PWD) modules

clean:
#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C /home/dilbert/xenomai/linux-2.6-denx/ M=$(PWD) clean
