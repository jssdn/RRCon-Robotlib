/**
    @file platform_io.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Specific high-level functions for the control of the GPIO peripherals in the RRcon Platform
    
    @author Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
 
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
    
    @note Interrupts not operational due to limitations in gpio.c

    @version 0.4-Xenomai       
    
*/

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

/**
* @brief Wrapper for General Inputs GPIO initialization
*
* @param fisr Pointer to the Interrupt Service Routine (NULL if no interrupt support)
* @return 0 on success. Otherwise error. 
*
* Initializes the General Inputs GPIO peripheral
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int pio_init_geninputs(void (*fisr)(void*))
{
    int err; 
    
    err = gpio_init(&pio_geninputs, GENERAL_INPUTS_BASE, GENERAL_INPUTS_END, 
		    GENERAL_INPUTS_NUM_OF_CHAN , GPIO_FLAGS_INPUT | GPIO_IRQ_CHANNEL1, GENERAL_INPUTS_IRQ_NO, 
		    fisr, GENERAL_INPUTS_IRQ_PRIO); 
		    
    gpio_set_dir(&pio_geninputs,~0,0,~0); // all inputs
    
    return err; 
}

/**
* @brief Wrapper for General Outputs GPIO initialization
*
* @return 0 on success. Otherwise error. 
*
* Initializes the General Outputs GPIO peripheral
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int pio_init_genoutputs()
{
    int err; 
    
    err = gpio_init(&pio_genoutputs, GENERAL_OUTPUTS_BASE, GENERAL_OUTPUTS_END, 
		    GENERAL_INPUTS_NUM_OF_CHAN, GPIO_FLAGS_OUTPUT, 0, 
		    0, 0 );     
		    
    gpio_set_dir(&pio_genoutputs,~0,0,0); // all outputs
    
    return err; 
}

/**
* @brief Wrapper for General Inputs GPIO initialization
*
* @param fisr Pointer to the Interrupt Service Routine (NULL if no interrupt support)
* @return 0 on success. Otherwise error. 
*
* Initializes the FPGA-GPIO peripheral
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int pio_init_fpgagpio(void (*fisr)(void*))
{
    int err;
    
    err = gpio_init(&pio_fpgagpio, FPGA_GPIO8_BASE, FPGA_GPIO8_END, 
		    FPGA_GPIO8_NUM_OF_CHAN , GPIO_FLAGS_INPUT | GPIO_FLAGS_OUTPUT, FPGA_GPIO8_IRQ_NO, 
		    fisr, FPGA_GPIO8_IRQ_PRIO);

    gpio_set_dir(&pio_fpgagpio,~0,0,~0); // all inputs by default

    return err; 
}

/**
* @brief Wrapper for all plataform IO initialization
*
* @param isr_ginputs Pointer to the Interrupt Service Routine for the General Inputs peripheral (NULL if no interrupt support)
* @param isr_fpga Pointer to the Interrupt Service Routine for the GPIO-FPGA peripheral (NULL if no interrupt support)
* @return 0 on success. Otherwise error. 
*
* Initializes the General Inputs, General Outputs and FPGA-GPIO peripherals
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

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

/**
* @brief Wrapper for all plataform IO cleaning
*
* @return 0 on success. Otherwise error. 
*
* Cleans the General Inputs, General Outputs and FPGA-GPIO peripherals
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

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

/**
* @brief Write to the 4 led bar
* 
* @param value Value to write ( 0x0 - 0xf )
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int pio_write_leds4(unsigned value)
{
     return gpio_write(&pio_genoutputs, GENERAL_OUTPUTS_LED4_MASK, GENERAL_OUTPUTS_LED4_SHIFT, 0, value);
}

/**
* @brief Write to the arrow positions leds
* 
* @param value Value to write ( 0x0 - 0x1f )
* @return 0 on success. Otherwise error. 
*
* @note Center ( bit 0 ) / Left ( 2 ) / Down ( 3 ) / Right ( 4 ) / Up ( 5 )
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int pio_write_ledspos(unsigned value)
{
    return gpio_write(&pio_genoutputs, GENERAL_OUTPUTS_LEDPOS_MASK, GENERAL_OUTPUTS_LEDPOS_SHIFT, 0, value);
}

/**
* @brief Read from the arrow positions buttons
* 
* @param value Read value
* @return 0 on success. Otherwise error. 
*
* @note Center ( bit 0 ) / Left ( 2 ) / Down ( 3 ) / Right ( 4 ) / Up ( 5 )
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int pio_read_buttons(unsigned* ret)
{
    return gpio_read(&pio_geninputs, GENERAL_INPUTS_PUSHBUT_MASK,GENERAL_INPUTS_PUSHBUT_SHIFT, 0, ret);        
}

/**
* @brief Read from the bumpers
* 
* @param ret Read value
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int pio_read_bumpers(unsigned* ret)
{
    return gpio_read(&pio_geninputs, GENERAL_INPUTS_BUMPERS_MASK,GENERAL_INPUTS_BUMPERS_SHIFT, 0, ret);        
}

/**
* @brief Read from the FPGA-GPIO port
* 
* @param ret Read value
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int pio_read_fpgagpio(unsigned* ret)
{
//     return gpio_read(&pio_fpgagpio, FPGA_GPIO8_MASK,0, 0, ret);
    return 0;
}

/**
* @brief Write to the FPGA-GPIO port
* 
* @param value Data to write into the port
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

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

/**
* @brief Unsafe access functions for muxed IO
* 
* @note This functions will be removed quickly
*
*/ 

inline int pio_write_go_all(unsigned value,unsigned off)
{
    return gpio_write(&pio_genoutputs, ~0,0, off, value);
}

/**
* @brief Unsafe access functions for muxed IO
* 
* @note This functions will be removed quickly
*
*/ 

inline int pio_read_gi_all(unsigned *ret,unsigned off)
{
    return gpio_fast_read(&pio_geninputs, off, ret);
}
