/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: hwservos.c Servo interface for hardware servo controller
    License: Licensed under GPL2.0 
  
    Note: Better created as a Thread
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
#include <time.h>

#include "busio.h"
#include "gpio.h"
#include "hwservos.h"


volatile int* hwservos_basep;

int map_servos()
{
    if( mapio_region(&hwservos_basep, HWSERVOS_BASE, HWSERVOS_END) < 0 )
	return -1;

    return 0;
}

int unmap_servos()
{
    int i; 
    
    for( i = 0 ; i < 4 ; i++ )
    {
      *(hwservos_basep + i) = 0x00 ; 
    }
    
    if( unmapio_region(&hwservos_basep, HWSERVOS_BASE, HWSERVOS_END) < 0 )
	return -1;
    return 0;
}

void servo_set_pos(enum servos s, unsigned int val) 
{
    // sanity check
    if(val < HWSERVOS_TIME_MIN_ANGLE ) val = HWSERVOS_TIME_MIN_ANGLE ; 
    else if (val > HWSERVOS_TIME_MAX_ANGLE ) val = HWSERVOS_TIME_MAX_ANGLE ; 
    *(hwservos_basep + s) = val | 0x80000000 ; 
}

inline void servo_enable(enum servos s) 
{
    *(hwservos_basep + s) = *(hwservos_basep + s) | 0x80000000 ; 
}

inline void servo_disable(enum servos s) 
{
    *(hwservos_basep + s) = *(hwservos_basep + s) & ~0x80000000 ; 
}
