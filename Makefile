all:
	gcc -DMODULE -D__KERNEL__ -O2 -c ring.c
	gcc test_ring.c -o test_ring

test: all
	-rmmod ring
	insmod ring
	-mknod /dev/ring0 c 114 0
	-mknod /dev/ring1 c 114 1
	./test_ring

clean:
	rm -rf ring.o test_ring
	-rm -rf /dev/ring*
	-rmmod ring
