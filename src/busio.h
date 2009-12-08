/**
    @file busio.h
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief [HEADER] Low-level Direct-IO allocations    
    
*/

#ifndef __BUSIO_H__
#define __BUSIO_H__

volatile void * ioremap(unsigned long physaddr, unsigned size);

int iounmap(volatile void *start, size_t length);

int mapio_region(volatile int** basep, long unsigned int base, long unsigned int end );

int unmapio_region(volatile int** basep, long unsigned int base, long unsigned int end );

#endif
