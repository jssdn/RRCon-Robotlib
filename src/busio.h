#ifndef __BUSIO_H__
#define __BUSIO_H__

// extern inline void out_8(volatile unsigned char *addr, unsigned val)

volatile void * ioremap(unsigned long physaddr, unsigned size);

int iounmap(volatile void *start, size_t length);

int mapio_region(volatile int** basep, long unsigned int base, long unsigned int end );

int unmapio_region(volatile int** basep, long unsigned int base, long unsigned int end );

#endif
