#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "ring_ioctl.h"

int main(void) {
	int ring0 = open("/dev/ring0", O_RDWR);
	int ring1 = open("/dev/ring1", O_RDWR);
	
	if(ring0 < 0 || ring1 < 0 ) {
		perror("Nie można otworzyć urządzenia pierścieniowego");
		exit(EXIT_FAILURE);
	}
	
	printf("Rozmiar buforu urzadzenia pierscieniowego 0: %d\n", ioctl(ring0, RING_IOR_GETSIZE));
	printf("Rozmiar buforu urzadzenia pierscieniowego 1: %d\n", ioctl(ring1, RING_IOR_GETSIZE));
    printf("Zmiana rozmiaru buforu urzadzenia pierscieniowego 1 na 1337 przy uzyciu ioctl...\n", ioctl(ring1, RING_IOR_GETSIZE));
	ioctl(ring1, RING_IOW_SETSIZE, 1337);
	printf("Rozmiar buforu urzadzenia pierscieniowego 0: %d\n", ioctl(ring0, RING_IOR_GETSIZE));
	printf("Rozmiar buforu urzadzenia pierscieniowego 1: %d\n", ioctl(ring1, RING_IOR_GETSIZE));
    return 0;
}
