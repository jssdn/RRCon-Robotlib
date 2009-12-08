/**
    @file platform_io.h
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief [HEADER] Specific high-level functions for the control of the GPIO peripherals in the RRcon Platform
    
*/

#ifndef __PLATFORM_IO_H__
#define __PLATFORM_IO_H__

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

/* Separate initializations for the devices */
int pio_init_geninputs(void (*fisr)(void*));

int pio_init_genoutputs();

int pio_init_fpgagpio(void (*fisr)(void*));

/* Joint functions for automated init of all devices */
int pio_init_all(void (*isr_ginputs)(void*), void (*isr_fpga)(void*));

int pio_clean_all();

/* Helpful easy-to-read high-level functions for gpio device read */

/* LED BAR 4 bits */
inline int pio_write_leds4(unsigned value);

/* ARROW POSITION LEDS */
inline int pio_write_ledspos(unsigned value);

/* ARROW POSITION BUTTONS */
inline int pio_read_buttons(unsigned* ret);

inline int pio_read_bumpers(unsigned* ret);

/* FPGA_GPIO8 */
inline int pio_read_fpgagpio(unsigned* ret);

inline int pio_write_fpgagpio(unsigned value);

inline int pio_write_fpgagpio_tristate(unsigned value);

// TODO: Remove. Unsafe functions

inline int pio_write_go_all(unsigned value,unsigned off);

inline int pio_read_gi_all(unsigned *ret,unsigned off);

#endif
