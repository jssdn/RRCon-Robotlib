/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: platform_io.c Specific high-level functions for the control of 
	                the gpio peripherals in the RRcon Platform
    License: Licensed under GPL2.0 

    Copyright (C) Jorge Sánchez de Nova
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

#include "platform_io.h"
#include "dev_mmaps_parms.h"
#include "gpio.h"
#include "util.h"

GPIO pio_geninputs; // Buttons / BUMPERS / ADC EOC / ACC_INT 
GPIO pio_genoutputs; // LEDS4 / LEDs_Position
GPIO pio_fpgagpio; // 8 general purpouse bidirectional signals

int pio_init_geninputs(void (*fisr)(void*))
{
    int err; 
    
    err = gpio_init(&pio_geninputs, GENERAL_INPUTS_BASE, GENERAL_INPUTS_END, 
		    GENERAL_INPUTS_NUM_OF_CHAN , GPIO_FLAGS_INPUT | GPIO_IRQ_CHANNEL1, GENERAL_INPUTS_IRQ_NO, 
		    fisr, GENERAL_INPUTS_IRQ_PRIO); 
		    
    gpio_set_dir(&pio_geninputs,~0,0,~0); // all inputs
    
    return err; 
}

int pio_init_genoutputs()
{
    int err; 
    
    err = gpio_init(&pio_genoutputs, GENERAL_OUTPUTS_BASE, GENERAL_OUTPUTS_END, 
		    GENERAL_INPUTS_NUM_OF_CHAN, GPIO_FLAGS_OUTPUT, 0, 
		    0, 0 );     
		    
    gpio_set_dir(&pio_genoutputs,~0,0,0); // all outputs
    
    return err; 
}

int pio_init_fpgagpio(void (*fisr)(void*))
{
    int err;
    
    err = gpio_init(&pio_fpgagpio, FPGA_GPIO8_BASE, FPGA_GPIO8_END, 
		    FPGA_GPIO8_NUM_OF_CHAN , GPIO_FLAGS_INPUT | GPIO_FLAGS_OUTPUT, FPGA_GPIO8_IRQ_NO, 
		    fisr, FPGA_GPIO8_IRQ_PRIO);

    gpio_set_dir(&pio_fpgagpio,~0,0,~0); // all inputs by default

    return err; 
}

int pio_init_all(void (*isr_ginputs)(void*), void (*isr_fpga)(void*))
{
    int res ; 
    
    if( (res = pio_init_geninputs(isr_ginputs)) <0 )
	    goto uninit_any;
    
    if( (res = pio_init_genoutputs()) < 0 )
	    goto uninit_go_fpga;
    
/*    if( (res = pio_init_fpgagpio(isr_fpga)) < 0 )
	    goto uninit_fpga;*/
    
    return 0;
    
// uninit_fpga:    
//     gpio_clean(&pio_geninputs);
//     gpio_clean(&pio_genoutputs);
//     util_pdbg(DBG_WARN, "PIO: GPIO FPGA_GPIO8 could not be initialized. Cleaning previosly initialized. Error:%d\n", res);
uninit_go_fpga:    
    gpio_clean(&pio_geninputs);
    util_pdbg(DBG_WARN, "PIO: GPIO \"General Outputs\" could not be initialized. Cleaning previosly initialized. Error:%d\n", res);
uninit_any:    
    util_pdbg(DBG_WARN, "PIO: GPIO \"General Inputs\" could not be initialized. Error:%d\n", res);
    return res;
}

int pio_clean_all()
{
    int res; 
    
    if( (res = gpio_clean(&pio_geninputs)) <0 )
	    goto uncleaned_all;
    
    if( (res = gpio_clean(&pio_genoutputs)) < 0 )
	    goto uncleaned_gout;
    
/*    if( (res = gpio_clean(&pio_fpgagpio)) < 0 )
	    goto uncleaned_fpga;*/
    
    return 0; 
    
uncleaned_all:
   util_pdbg(DBG_WARN, "PIO: Error cleaning \"General Inputs\" GPIO. None could be cleaned. Error:%d \n", res);
uncleaned_gout:
   util_pdbg(DBG_WARN, "PIO: Error cleaning \"General Outputs\". \"General Inputs\" could be cleaned.\n ", res);
// uncleaned_fpga:
//    util_pdbg(DBG_WARN, "PIO: Error cleaning GPIOs. \"General Inputs/Outputs\" could be cleaned. \n", res);

   return res; 
}

/* Helpful easy-to-read high-level functions for gpio device read */

/* LED BAR 4 bits */
inline int pio_write_leds4(unsigned value)
{
     return gpio_write(&pio_genoutputs, GENERAL_OUTPUTS_LED4_MASK, GENERAL_OUTPUTS_LED4_SHIFT, 0, value);
}

/* ARROW POSITION LEDS */
/*
    Bits: 
		UP
		|
		5
		
	Left-2	|1|  4-Right	
              Center
		3
		|
		Down
*/
inline int pio_write_ledspos(unsigned value)
{
    return gpio_write(&pio_genoutputs, GENERAL_OUTPUTS_LEDPOS_MASK, GENERAL_OUTPUTS_LEDPOS_SHIFT, 0, value);
}

/* ARROW POSITION BUTTONS */
/*
    Bits: 
		UP
		|
		5
		
	Left-2	|1|  4-Right	
              Center
		3
		|
		Down
*/
inline int pio_read_buttons(unsigned* ret)
{
    return gpio_read(&pio_geninputs, GENERAL_INPUTS_PUSHBUT_MASK,GENERAL_INPUTS_PUSHBUT_SHIFT, 0, ret);        
}

inline int pio_read_bumpers(unsigned* ret)
{
    return gpio_read(&pio_geninputs, GENERAL_INPUTS_BUMPERS_MASK,GENERAL_INPUTS_BUMPERS_SHIFT, 0, ret);        
}

/* FPGA_GPIO8 */
inline int pio_read_fpgagpio(unsigned* ret)
{
//     return gpio_read(&pio_fpgagpio, FPGA_GPIO8_MASK,0, 0, ret);
    return 0;
}

inline int pio_write_fpgagpio(unsigned value)
{
//     return gpio_write(&pio_fpgagpio, FPGA_GPIO8_MASK,0, 0, value);
    return 0;
}

inline int pio_write_fpgagpio_tristate(unsigned value)
{
//     return gpio_write(&pio_fpgagpio, FPGA_GPIO8_MASK,0, GPIO_TRISTATE_OFFSET, value);
    return 0;
}

/* Unsafe access functions for muxed IOs */
//TODO: REMOVE! Unsafe functions
inline int pio_write_go_all(unsigned value,unsigned off)
{
    return gpio_write(&pio_genoutputs, ~0,0, off, value);
}

inline int pio_read_gi_all(unsigned *ret,unsigned off)
{
    return gpio_fast_read(&pio_geninputs, off, ret);
}
