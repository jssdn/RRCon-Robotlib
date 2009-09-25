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

#include "gpio_maps.h"
#include "gpio.h"
#include "util.h"
// 

extern GPIO pio_geninputs; // Buttons / ADC EOC / ACC_INT / BUMPERS
extern GPIO pio_genoutputs; // LCD / LEDS4 / LEDs_Position / USB_RESET
extern GPIO pio_fpgagpio; // 8 general purpouse bidirectional signals

/* Separate initializations for the devices */
inline int pio_init_geninputs(void (*fisr)(void*));

inline int pio_init_genoutputs();

inline int pio_init_fpgagpio(void (*fisr)(void*));

/* Joint functions for automated init of all devices */
int pio_init_all(void (*isr_ginputs)(void*), void (*isr_fpga)(void*));

int pio_clean_all();

/* Helpful easy-to-read high-level functions for gpio device read */

/* LED BAR 4 bits */
inline int pio_read_leds4(unsigned* ret);

inline int pio_write_leds4(unsigned value);

/* ARROW POSITION LEDS */
inline int pio_read_ledspos(unsigned* ret);

inline int pio_write_ledspos(unsigned value);

/* ARROW POSITION BUTTONS */
inline int pio_read_buttons(unsigned* ret);
/* FPGA_GPIO8 */
inline int pio_read_fpgagpio(unsigned* ret);

inline int pio_write_fpgagpio(unsigned value);

inline int pio_write_fpgagpio_tristate(unsigned value);

#endif
