obj-m += flipper.o

all:
	make -C ./buildroot/buildroot-2023.02.2/output/build/linux-6.1.38 M=$(PWD) modules
clean:
	make -C ./buildroot/buildroot-2023.02.2/output/build/linux-6.1.38 M=$(PWD) clean