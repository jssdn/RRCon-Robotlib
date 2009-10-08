#ifndef __HWSERVOS__H__
#define __HWSERVOS__H__

#include "dev_mmaps_parms.h"
#include <native/mutex.h>

typedef struct{
    unsigned long base_add; 	// Physical memory address where to map the device  (beginning)
    unsigned long end_add;      // Physical memory address where to map the device (end)
    volatile int* vadd;         // virtual address where device is mapped
    unsigned* values;		//Latched values of the Servos. Allocated during initialization
    unsigned num_of; 		//Number of servos in device
    RT_MUTEX mutex; 
} HWSERVOS; 

int hwservos_init(HWSERVOS* servo, unsigned long base_add, unsigned long end_add, unsigned num_of);
int hwservos_clean(HWSERVOS* servo);

int hwservos_set_pos(HWSERVOS* servo, unsigned num, unsigned value);
/* NOTE: Returned value IS NOT BASED IN HW feedback, but in latched values */
int hwservos_get_pos(HWSERVOS* servo, unsigned num, unsigned* ret);

int hwservos_enable(HWSERVOS* servo, unsigned num);
int hwservos_disable(HWSERVOS* servo, unsigned num);

#endif
