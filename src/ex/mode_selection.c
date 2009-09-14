/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: mode_selection.c Simple boottime selection mode interface

    returns: 1 - Development mode
             2 - Execution mode 
             other - Errors

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/types.h>

#include "gpio.h"
#include "lcd_proc.h"


void blink( uint8_t val, long int time)
{
    // Blinking to show the user the Buttons that should press
    write_led_dir(val);
    usleep(time); 
    write_led_dir(0x0);
    usleep(time); 
    write_led_dir(val);

}

int main(int argc, char** argv)
{
    uint8_t keep = 1;
    uint8_t sel_flag = 0;

    printf("Mapping GPIO...\n");

    if ( map_gpio() < 0 ){
        printf("Error!\n");
        exit(-1);
    }

    lcd_write_text("Select mode:    ", 16, LINE1);
    lcd_write_text("Press left/right", 16, LINE2);

    blink(0x0a, 125000); // left&right

    printf("Testing buttons... Press left+right to exit.\n");

    while(keep)
    {
        switch(read_buttons()) {
            case 0x02: // right
                lcd_write_text("Pres center!    ", 16, LINE1);
                lcd_write_text("Dev mode ON     ", 16, LINE2);
                sel_flag = 1;
                blink(0x10, 125000); 
                break;

            case 0x08: // left 
                lcd_write_text("Pres center!    ", 16, LINE1);
                lcd_write_text("Exec mode ON    ", 16, LINE2);
                sel_flag = 2;
                blink(0x10, 125000); 
                break;

            case 0x10:
                if(sel_flag){
                    write_led_dir(0);
                    unmap_gpio();
                    // TODO:Dirty! integrate CMD functions
                    lcd_write_text("                ", 16, LINE1);
                    printf("Mode flag %d selected.\n",sel_flag);
                    return sel_flag;
                }
                break;
            default:
                break;
        }
    }

    write_led_dir(0);
    unmap_gpio();
    return -1;
}
