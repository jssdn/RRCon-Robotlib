/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: openloop_motors.c Functions for the basic DC motor control with quadrature enconder
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

#include "busio.h"
#include "openloop_motors.h"

volatile int* motors_basep;
volatile int* qenc_basep;

int map_motors()
{
    if( mapio_region(&motors_basep, MOTORS_BASE, MOTORS_END) < 0 )
	return -1;

    return 0;
}

int unmap_motors()
{
    if( unmapio_region(&motors_basep, MOTORS_BASE, MOTORS_END) < 0 )
	return -1;

    return 0;
}

inline void oloopm_set_freq_div( enum motors m, unsigned n)
{
    n = (n > OLOOPM_MAX_FREQ_DIV) ? OLOOPM_MAX_FREQ_DIV : n ;

    *(motors_basep + (m<<1) + 1) = n ; 
}

inline unsigned int oloopm_read_freq_div(enum motors m)
{
    return *(motors_basep + (m<<1) + 1); 
}

inline void oloopm_set_speed( enum motors m, int speed)
{
    // Maximum speed ( saturation )
    speed = (speed > OLOOP_MAX_SPEED)? OLOOP_MAX_SPEED : speed ;
    speed = (speed < -OLOOP_MAX_SPEED)? -OLOOP_MAX_SPEED : speed ;

    *(motors_basep + (m<<1)) = speed ; 
}

inline unsigned int oloopm_read_speed(enum motors m)
{
    return *(motors_basep + (m<<1)); 
}

int map_qenc()
{
    if( mapio_region(&qenc_basep, QENC_BASE, QENC_END) < 0 )
        return -1; 
    return 0;
}

int unmap_qenc()
{
    if( unmapio_region(&qenc_basep, QENC_BASE, QENC_END) < 0 )
        return -1; 
    return 0;
}

inline void oloopenc_setzero(enum motors m)
{
  *(qenc_basep + m) = 1; 
  *(qenc_basep + m) = 0; 
}

inline signed int oloopenc_read_pulsecount(enum motors m)
{
    return *(qenc_basep + m); 
}



