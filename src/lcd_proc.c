/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: lcd_proc.c Interface functions for the xilinx_lcd.ko kernel module. 
          Communication through /proc/xilinx_lcdXXXXX interfaces

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/types.h>

#include "gpio.h"
#include "lcd_proc.h"

int lcd_write_text(char* str, uint8_t len, enum lcd_write_mode mode)
{
    int tmp,res; 
    FILE* xilinx_lcd;

//     len = len>16?len:16; // Not needed, the kernel module will handle this

    // Not very elegant, but it simplifies the LCD writing in one function without much overhead
    switch(mode){
        case LINE1:
            if( (xilinx_lcd = fopen(LCD_XILINX_L1_IFACE , "w")) == NULL )
                return -1; 
            break;
        case LINE2:
            if( (xilinx_lcd = fopen(LCD_XILINX_L2_IFACE , "w")) == NULL )
                return -1; 
            break;
        case TEXT:
        default:
            if( (xilinx_lcd = fopen(LCD_XILINX_LINE_IFACE , "w")) == NULL )
                return -1; 
            break;
    }

    if( ( res = fwrite(str, len , sizeof(char), xilinx_lcd) ) < 0 )
        return -2;

    if(!(tmp = fclose( xilinx_lcd )))
        return tmp;

    return res;
}


// TODO: Userspace CMD functions
