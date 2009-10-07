#ifndef __HWSERVOS__H__
#define __HWSERVOS__H__

#include "dev_mmaps_parms.h"
#include <native/mutex.h>

typedef struct{
    unsigned values[HWSERVOS_NUM_OF]; //Latched values of the Servos
    unsigned long base_add; 	      // Physical memory address where to map the device  (beginning)
    unsigned long end_add;            // Physical memory address where to map the device (end)
    volatile int* vadd;               // virtual address where device is mapped
    RT_MUTEX mutex; 
} HWSERVOS; 

int map_servos(HWSERVOS* servo);

int unmap_servos(HWSERVOS* servo);

int servo_set_pos(HWSERVOS* servo, unsigned num, unsigned value);

/* NOTE: Returned value IS NOT BASED IN HW feedback, but in latched values */
int servo_read_pos(HWSERVOS* servo, unsigned num, unsigned* ret); 

int servo_enable(HWSERVOS* servo, unsigned num);

int servo_disable(HWSERVOS* servo, unsigned num);

#endif
