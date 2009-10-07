/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: gpio.c Functions for the Xilinx XPS GPIO cores in userspace with IRQ support.
    License: Licensed under GPL2.0 

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

*  ******************************************************************************* **/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <errno.h>

//Xenomai
#include <native/task.h>
#include <native/mutex.h>
#include <native/intr.h>
//--

#include "busio.h"
#include "dev_mmaps_parms.h"
#include "gpio.h"
#include "util.h"
// 

/*TODO: 
    GPIO as input -> writing to IO doesn't make any effect
    GPIO as ouput -> reading from IO doesn't make any effect
*/

//TODO: Non-blocking write functions

/*
    Writes to GPIO device
    gpio-> Gpio device
    mask-> Mask for muxed gpios
    shift-> Shift for muxed gpios
    offset-> Offset to the address in the gpio
    value-> Value to write
*/
int gpio_write(GPIO* gpio, unsigned mask, unsigned shift, unsigned offset, unsigned value)
{
    int err; 

    if( (err = rt_mutex_acquire(&(gpio->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "GPIO: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
 
   // check flags for input only?
/*     util_pdbg(DBG_DEBG, "Writing 0x%x to 0x%x vadd 0x%x off 0x%x\n",(value << shift) & mask, gpio->vadd + offset, gpio->vadd, offset);*/
    *(gpio->vadd + offset) = (value << shift) & mask; 

    if( (err = rt_mutex_release(&(gpio->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "GPIO: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }

    return 0; 
}

/*
    Writes to GPIO device
    gpio-> Gpio device
    mask-> Mask for muxed gpios(set to all ones if not muxed (~0))
    shift-> Shift for muxed gpios(set to 0 if not muxed)
    offset-> Offset to the address in the gpio(set to 0 if writing directly to IO is desired)
    ret-> return value
*/

int gpio_read(GPIO* gpio, unsigned mask, unsigned shift, unsigned offset, unsigned* ret)
{
    int err; 
    //TODO: check offset?
    if( (err = rt_mutex_acquire(&(gpio->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;

    *ret = (*(gpio->vadd + offset) & mask) >> shift; 
/*    util_pdbg(DBG_DEBG, "Reading 0x%x from 0x%x\n",*ret, gpio->vadd + offset, gpio->vadd, offset);    */
    if( (err = rt_mutex_release(&(gpio->mutex))) < 0 )
	return err; 

    return 0; 
}

/* Quick functions for IRQ handling inside the device */
int gpio_irq_enable_global(GPIO* gpio)
{
    return gpio_write(gpio, ~0x0, 0, GPIO_GIE_OFFSET , GPIO_GIE_MASK);
}

int gpio_irq_disable_global(GPIO* gpio)
{
    return gpio_write(gpio, ~0x0, 0, GPIO_GIE_OFFSET , 0x0);
}

/* By default assumes n = 1 */
int gpio_irq_enable_channel(GPIO* gpio,int n)
{
    int err; 
    unsigned ret;
    
    if( (err = gpio_read(gpio, ~0x0, 0, GPIO_IER_OFFSET , &ret)) < 0 )
	return err; 

    if( (n == 2) && (gpio->num_of_channels == 2 )){
	return gpio_write(gpio, ~0x0, 0, GPIO_IER_OFFSET , ret | GPIO_IER_CHANNEL2_MASK);
    }
	
    return gpio_write(gpio, ~0x0, 0, GPIO_IER_OFFSET , ret | GPIO_IER_CHANNEL1_MASK);
}

/* By default assumes n = 1 */
int gpio_irq_disble_channel(GPIO* gpio,int n)
{
    int err; 
    unsigned ret;

    if( (err = gpio_read(gpio, ~0x0, 0, GPIO_IER_OFFSET , &ret)) < 0 )
	return err; 

    if( n == 2 && gpio->num_of_channels == 2 ){
	return gpio_write(gpio, ~0x0, 0, GPIO_IER_OFFSET , ret & GPIO_IER_CHANNEL1_MASK);
    }
	
    return gpio_write(gpio, ~0x0, 0, GPIO_IER_OFFSET , ret & GPIO_IER_CHANNEL2_MASK);
}

/* 
   Reads and toggles the Interrupt Status Register 
   Toggling is performed by writing 1 into the ISR
*/
int gpio_irq_isr_checkandtoggle_channel(GPIO* gpio,int n, unsigned* ret )
{
    int err; 

    if( (err = gpio_read(gpio, ~0x0, 0, GPIO_ISR_OFFSET , ret)) < 0 )
	return err; 

    return gpio_write(gpio, ~0x0, 0, GPIO_IER_OFFSET , *ret);    
}



//so far only for 1channel gpios
int gpio_init(GPIO* gpio, 
	      long unsigned int base_add, 
	      long unsigned int end_add, 
	      int num_of_channels, 
	      char flags, 
	      unsigned irqno,
	      void (*fisr)(void*),
	      int irq_prio)
{
    int err; 
    unsigned ret;

    util_pdbg(DBG_INFO, "GPIO: Initializing GPIO:\n");

    // map
    if( (err = mapio_region(&(gpio->vadd), base_add,end_add)) < 0 )
        return err; 

    gpio->base_add = base_add;
    gpio->end_add = end_add;
    // flags
    gpio->flags = flags; 
    gpio->num_of_channels = num_of_channels == 2 ? 2: 1;

    // Mutex init
    if( (err = rt_mutex_create(&(gpio->mutex), NULL)) < 0 ){
	    util_pdbg(DBG_WARN, "GPIO: Error rt_mutex_create: %d\n", err);
	    perror(0);
	    return err;
    } 

//NOTE: This should go somewhere else
//     // tri-state 
//     if( (err =  gpio_write(gpio, ~0x0,0, GPIO_TRISTATE_OFFSET, tristate)) < 0){
// 	 util_pdbg(DBG_WARN, "Error writing to GPIO in address 0x%x. Error: %d \n", gpio->vadd, err);
// 	 return err; 
//     }

     // IRQ init
    if (fisr != NULL && ((flags & GPIO_IRQ_CHANNEL1) || (flags & GPIO_IRQ_CHANNEL2))  ) { //flags activated and isr present
	if( (err = rt_intr_create(&(gpio->intr_desc), "GPIO IRQ", irqno, I_NOAUTOENA)) < 0 ){
	    util_pdbg(DBG_WARN, "GPIO: Cannot create interrupt for GPIO rt_intr_create=%i\n", err);
	    return err;
	}
	    
	rt_intr_enable (&(gpio->intr_desc));

	if( (err = rt_task_spawn(&(gpio->interrupt_task), "Int", 0, irq_prio, 0,fisr, (void*)&(gpio->intr_desc))) < 0){
	    util_pdbg(DBG_WARN, "GPIO: Cannot Spawn ISR for GPIO. err = %d\n", err);
	    return err; 
	}

	// Enable the interrupts inside the device
	if( (err = gpio_irq_enable_global(gpio)) < 0){
	    return err; 
	}

	switch(flags&(GPIO_IRQ_CHANNEL1|GPIO_IRQ_CHANNEL2)) // Activate IRQs only for the desired channels
	{
	    case GPIO_IRQ_CHANNEL1:
		if( (err = gpio_irq_enable_channel(gpio,1)) < 0 )
		    return err; 
		break;
	    case GPIO_IRQ_CHANNEL2:
		if( (err = gpio_irq_enable_channel(gpio,2)) < 0 )
		    return err; 
		break;
	    case GPIO_IRQ_CHANNEL1|GPIO_IRQ_CHANNEL2:
		if( (err = gpio_irq_enable_channel(gpio,1)) < 0 )
		    return err; 

		if( (err = gpio_irq_enable_channel(gpio,2)) < 0 )
		    return err; 

		break;
	}
    
	#if DBG_LEVEL == 5
	gpio_read(gpio, ~0x0, 0, GPIO_GIE_OFFSET , &ret);
	util_pdbg(DBG_DEBG,"GIE:0x%x\t",ret);	
	gpio_read(gpio, ~0x0, 0, GPIO_IER_OFFSET , &ret);
	util_pdbg(DBG_DEBG,"IER:0x%x\n",ret);	
	#endif
    }	

    gpio->isr = fisr;
    return 0; 
//TODO: Error clean through tag cleaning
}

int gpio_clean(GPIO* gpio)
{
    int err; 
    // unmap
    util_pdbg(DBG_DEBG, "GPIO: Cleaning GPIO...\n");
    if ( (err = unmapio_region(&(gpio->vadd), gpio->base_add, gpio->end_add)) < 0 ){
	util_pdbg(DBG_WARN, "GPIO: GPIO couldn't be unmapped at virtual= 0x%x . Error : %d \n", &(gpio->vadd), err);
	return err; 
    }
    // delete mutex
    if( ( err = rt_mutex_delete(&(gpio->mutex)) ) < 0 ){
	util_pdbg(DBG_WARN, "GPIO: GPIO Mutex cannot be deleted \n");
	return err; 
    }
    // TODO: check in another way
    if(!gpio->isr){
	if( ( err = rt_intr_delete(&(gpio->intr_desc))) < 0 ){
	    util_pdbg(DBG_WARN, "GPIO: Cannot delete IRQ\n");
	    perror(NULL);
	    return err; 
	}
    }
    return 0; 
}

