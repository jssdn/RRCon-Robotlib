/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: gpio.c Functions for the Xilinx XPS GPIO cores in userspace with IRQ support.
    License: Licensed under GPL2.0 

*  ******************************************************************************* */

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
#include <native/queue.h>
#include <native/intr.h>
//--

#include "busio.h"
#include "gpio_maps.h"
#include "gpio.h"
#include "util.h"
// 


int gpio_write(GPIO* gpio, unsigned mask, unsigned offset, unsigned value)
{
    int err; 

    //TODO: check offset?
    if( (err = rt_mutex_acquire(&(gpio->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;

    // check flags for input only?
    *(gpio->vadd + offset) = value & mask; 

    if( (err = rt_mutex_release(&(gpio->mutex))) < 0 )
	return err; 

    return 0; 
}

int gpio_read(GPIO* gpio, unsigned mask, unsigned offset, unsigned* ret)
{
    int err; 
    //TODO: check offset?
    if( (err = rt_mutex_acquire(&(gpio->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;
 
    ret = *(gpio->vadd + offset) & mask; 

    if( (err = rt_mutex_release(&(gpio->mutex))) < 0 )
	return err; 

    return 0; 
}

//so far only for 1channel gpios
int gpio_init(GPIO* gpio, 
	      long unsigned int base_add, 
	      long unsigned int end_add, 
	      unsigned tristate, 
	      char flags, 
	      unsigned irqno,
	      void (*fisr)(void*),
	      int irq_prio)
{
    int err; 
    
    // map
    if( (err = mapio_region(&(gpio->vadd), base_add,end_add)) < 0 )
        return err; 

    gpio->base_add = base_add;
    gpio->end_add = end_add;
    // flags
    gpio->flags = flags; 

    // tri-state
    if( (err =  gpio_write(gpio, ~0x0, GPIO_TRISTATE_OFFSET, tristate)) < 0){
	 util_pdbg(DBG_WARN, "Error writing to GPIO in address %d \n", gpio->vadd);
	 return err; 
    }

    // Mutex init
    if( (err = rt_mutex_create(&(gpio->mutex), "GPIO Reg Mutex")) < 0 ){
	    util_pdbg(DBG_WARN, "Error rt_mutex_create: %d\n", err);
	    return err;
    } 

     // IRQ init
    if (fisr != NULL) {
	if( (err = rt_intr_create(&(gpio->intr_desc), "GPIO IRQ", irqno, I_NOAUTOENA)) < 0 ){
	    util_pdbg(DBG_WARN, "Cannot create interrupt for GPIO \n" );
	    return err;
	}
	    
	util_pdbg(DBG_DEBG, "rt_intr_create=%i\n", err);
	rt_intr_enable (&(gpio->intr_desc));

	if( (err = rt_task_spawn(&(gpio->interrupt_task), "Int", 0, irq_prio, 0,fisr, NULL)) < 0){
	    util_pdbg(DBG_WARN, "Cannot Spawn ISR for GPIO. err = %d\n", err);
	    return err; 
	}		
    }
    gpio->isr = &fisr;

    return 0; 
}

int gpio_clean(GPIO* gpio)
{
    int err; 
    // unmap
    util_pdbg(DBG_DEBG, "Cleaning GPIO... ");
    // delete mutex
    if( ( err = rt_mutex_delete(&(gpio->mutex)) ) < 0 ){
	util_pdbg(DBG_WARN, "GPIO Mutex cannot be deleted \n");
	return err; 
    }
    // TODO: check in another way
    if(!gpio->isr){
	if( ( err = rt_intr_delete(&(gpio->intr_desc))) < 0 ){
	    util_pdbg(DBG_WARN, "Cannot delete IRQ\n");
	    return err; 
	}
    }
    util_pdbg(DBG_DEBG, "Done!\n", err);
    return 0; 
}

