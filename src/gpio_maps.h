#ifndef __GPIOMAPS_H__
#define __GPIOMAPS_H__
/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: Description for the mappings of the GPIO devices

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

#define DIRLED_BASE 0x81440000
#define DIRLED_END  0x8144ffff

#define LED4_BASE 0x81460000
#define LED4_END  0x8146ffff

#define BUTTONS_BASE 0x81420000
#define BUTTONS_END  0x8142ffff

#define BUMPERS_BASE 0x400E0000
#define BUMPERS_END  0x400Effff

#define FPGA_GPIO8_BASE 0x401E0000
#define FPGA_GPIO8_END 0x401E0000

#endif 
