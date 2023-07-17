CC = gcc
PROT = -static #-fPIE -fstack-protector-all -Wl,-z,relro,-z,now
CFLAGS = -std=gnu11 -Wall -Wextra -O1
TARGET = deploy

.PHONY : exploit

exploit : exploit.c io_uring.c cred.c
	$(CC) -o exploit $(CFLAGS) $(PROT) exploit.c io_uring.c cred.c

clean :
	rm -f exploit
