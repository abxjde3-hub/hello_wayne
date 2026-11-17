obj-m += hello_wayne.o
all:
	make -C ~/WSL2-Linux-Kernel M=$(PWD) modules
clean:
	make -C ~/WSL2-Linux-Kernel M=$(PWD) clean
