/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: gpio.c Functions for the basic GPIO devices
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
#include "gpio_maps.h"
#include "gpio.h"

volatile int* led4_base;
volatile int* dirled_base;
volatile int* buttons_base;
volatile int* bumpers_base;

int map_gpio()
{
    int res;

    if( (res = mapio_region(&led4_base, LED4_BASE, LED4_END)) < 0 ) 
        return res; 
    printf("led4_base 0x%x\n",led4_base);
    *(led4_base + GPIO_TRISTATE_OFFSET) = 0x00 ; // configured as outputs

    if( (res = mapio_region(&dirled_base, DIRLED_BASE,DIRLED_END)) < 0 ) 
        return res; 

    *(dirled_base + GPIO_TRISTATE_OFFSET) = 0x00 ; // configured as outputs

    if( (res = mapio_region(&buttons_base, BUTTONS_BASE,BUTTONS_END)) < 0 ) 
        return res; 

    *(buttons_base + GPIO_TRISTATE_OFFSET) = 0xff ; // configured as inputs

    if( (res = mapio_region(&bumpers_base, BUMPERS_BASE,BUMPERS_END)) < 0 ) 
        return res; 

    *(bumpers_base + GPIO_TRISTATE_OFFSET) = 0xff ; // configured as inputs

    return 0; 
}

int unmap_gpio()
{
    int res;

    if( (res = unmapio_region(&led4_base,LED4_BASE,LED4_END)) < 0 )
            return res;		

    if( (res = unmapio_region(&dirled_base,DIRLED_BASE,DIRLED_END)) < 0 )
            return res;		

    if( (res = unmapio_region(&buttons_base, BUTTONS_BASE,BUTTONS_END)) < 0 )
            return res;		

    return 0; 
}

inline void write_led4(uint8_t val)
{
    *led4_base = val & 0x0f;
}

inline void write_led_dir(uint8_t val)
{
    *dirled_base = val & 0x1f; 
}

// Polling
inline uint8_t read_buttons()
{
    return *buttons_base;
}

inline uint8_t read_bumpers()
{
  return *bumpers_base; 
}