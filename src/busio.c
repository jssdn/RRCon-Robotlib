/*
 *   Project: Robotics library for the Autonomous Robotics Development Platform  
 *   Code: busio.c Low-level direct-IO allocations
 *   Mods:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
 *   Original code from Stephane Fillod (C) 2003
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h> 
 
#ifdef __PPC__
extern inline void out_8(volatile unsigned char *addr, unsigned val)
{
        __asm__ __volatile__("stb%U0%X0 %1,%0; eieio" : "=m" (*addr) : "r" (val));
}
/* etc., cf asm/io.h */
#else
extern inline void out_8(volatile unsigned char *addr, unsigned val)
{
        *addr = val & 0xff;
}
#endif


volatile void * ioremap(unsigned long physaddr, unsigned size)
{
    static int axs_mem_fd = -1;
    unsigned long page_addr, ofs_addr, reg, pgmask;
    void* reg_mem = NULL;

    /*
     * looks like mmap wants aligned addresses?
     */
    pgmask = getpagesize()-1;
    page_addr = physaddr & ~pgmask;
    ofs_addr  = physaddr & pgmask;

    /*
     * Don't forget O_SYNC, esp. if address is in RAM region.
     * Note: if you do know you'll access in Read Only mode,
     *    pass O_RDONLY to open, and PROT_READ only to mmap
     */
    if (axs_mem_fd == -1) {
        axs_mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
        if (axs_mem_fd < 0) {
                perror("AXS: can't open /dev/mem");
                return NULL;
        }
    }

    /* memory map */
    reg_mem = mmap(
        (caddr_t)reg_mem,
        size+ofs_addr,
        PROT_READ|PROT_WRITE,
        MAP_SHARED,
        axs_mem_fd,
        page_addr
    );
    if (reg_mem == MAP_FAILED) {
        perror("AXS: mmap error");
        close(axs_mem_fd);
        return NULL;
    }

    reg = (unsigned long )reg_mem + ofs_addr;
    return (volatile void *)reg;
}

int iounmap(volatile void *start, size_t length)
{
    unsigned long ofs_addr;
    ofs_addr = (unsigned long)start & (getpagesize()-1);

    /* do some cleanup when you're done with it */
    return munmap((void*)start-ofs_addr, length+ofs_addr);
}

int mapio_region(volatile int** basep, long unsigned int baseadd, long unsigned int endadd)
{
    if( baseadd >= endadd )
    {
            printf( "Cannot allocate negative or zero size\n");
            return -EINVAL;
    }

    if( (*basep = (volatile int*) ioremap(baseadd, endadd - baseadd)) == NULL )
    {
            printf( "Cannot allocate memory at phy:0x%x-0x%x\n", (unsigned)baseadd, (unsigned)endadd); 
            return -EADDRNOTAVAIL;		
    }

    #ifdef DEBUGMODE
    printf( "Memory mapped at phy:0x%x-0x%x vrt=0x%x\n", (unsigned)baseadd, (unsigned)endadd, *basep); 
    #endif

    return 0;
}

inline int unmapio_region(volatile int** basep, long unsigned int baseadd, long unsigned int endadd)
{

    if( (iounmap(*basep, endadd - baseadd)) < 0 )
    {
                printf( "Cannot unmap memory at phy:0x%x-0x%x\n", (unsigned)baseadd, (unsigned)endadd); 
                return -ENXIO;		
    }

    return 0; 
}

