/**
    @file busio.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Low-level Direct-IO allocations    
    
    @author Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
    @author Based on code by Stephane Fillod (C) 2003
 
    @section LICENSE 
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
    
    @version 0.4-Xenomai

 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h> 
 
#include "util.h"

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

/**
* @brief Remaps a physical address into the virtual memory
*
* @param physaddr Physical base address of the peripheral mappable from the system bus
* @param size Addresable memory size
* @return Pointer to the virtual memory address
*/
volatile void* ioremap(unsigned long physaddr, unsigned size)
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
                util_pdbg(DBG_WARN, "ioremap: can't open /dev/mem\n");
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
	util_pdbg(DBG_WARN, "ioremap: mmap error\n");        
        close(axs_mem_fd);
        return NULL;
    }

    reg = (unsigned long )reg_mem + ofs_addr;
    return (volatile void *)reg;
}

/**
* @brief Unmaps the virtual address 
*
* @param start Virtual base address of the peripheral
* @param length Allocated memory size
* @return 0 on success. Otherwise error from munmap()
*/

int iounmap(volatile void *start, size_t length)
{
    unsigned long ofs_addr;
    ofs_addr = (unsigned long)start & (getpagesize()-1);

    /* do some cleanup when you're done with it */
    return munmap((void*)start-ofs_addr, length+ofs_addr);
}

/**
* @brief Wrapper function for ioremap() with error checking
*
* @see ioremap()
* @param basep Virtual base address of the peripheral
* @param baseadd Physical base address of the peripheral
* @param endadd Physical base address of the peripheral
* @return 0 on success. 
* @return -EADDRNOTAVAIL if address fails to be mapped
* @return -EINVAL if address sanity check fails
*/

int mapio_region(volatile int** basep, long unsigned int baseadd, long unsigned int endadd)
{
    if( baseadd >= endadd )
    {
	    util_pdbg(DBG_WARN, "Cannot allocate negative or zero size\n");        
            return -EINVAL;
    }

    if( (*basep = (volatile int*) ioremap(baseadd, endadd - baseadd)) == NULL )
    {
	util_pdbg(DBG_WARN, "Cannot allocate memory at phy:0x%x-0x%x\n", (unsigned)baseadd, (unsigned)endadd);
	return -EADDRNOTAVAIL;		
    }

    util_pdbg(DBG_DEBG, "Memory mapped at phy:0x%x-0x%x vrt=0x%x\n", (unsigned)baseadd, (unsigned)endadd, *basep); 

    return 0;
}

/**
* @brief Wrapper function for iounmap() with error checking
*
* @see iounmap()
* @param basep Virtual base address of the peripheral
* @param baseadd Physical base address of the peripheral
* @param endadd Physical base address of the peripheral
* @return 0 on success. 
* @return -ENXIO if virtual address cannot be unmapped
*/

int unmapio_region(volatile int** basep, long unsigned int baseadd, long unsigned int endadd)
{

    if( (iounmap(*basep, endadd - baseadd)) < 0 )
    {
	util_pdbg(DBG_WARN, "Cannot unmap memory at phy:0x%x-0x%x\n", (unsigned)baseadd, (unsigned)endadd); 
	return -ENXIO;		
    }

    return 0; 
}

