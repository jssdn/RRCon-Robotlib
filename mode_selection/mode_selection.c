/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author: Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: mode_selection.c Simple boottime selection mode interface

    returns: 1 - Development mode
             2 - Execution mode 
             other - Errors

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <fcntl.h>

//Xenomai
#include <native/task.h>
#include <native/pipe.h>
//-- 

/* Xenomai task variables */
#define STACK_SIZE 8192
#define MS_PRIO 5

#include "gpio.h"
#include "platform_io.h"
#include "util.h"

/* Platform devices */
GPIO gpio; 

/* Xenomai per-task variables */
RT_TASK ms_ptr;

/* Pipe */
#define LCD_PIPE_FILE "/proc/xenomai/registry/native/pipes/LCD_PIPE"
int lcd_pipe;


void blink( uint8_t val, long int time)
{
    // Blinking to show the user the Buttons that should press
    pio_write_ledspos(val);    
    __usleep(time); 
    pio_write_ledspos(0x00);    
    __usleep(time); 
    pio_write_ledspos(val);    
}

void main_task(void* cookie)
{
    int err;
    int keep = 1; 
    uint8_t sel_flag = 0;
    unsigned but; 
    
    if ( (lcd_pipe = open(LCD_PIPE_FILE, O_WRONLY)) < 0){
	perror(NULL);
	printf("Couldn't open LCD PIPE\n");
	exit(-1);
    }

    if( (err = pio_init_all(NULL, NULL)) < 0 ) {
	perror(NULL);
	util_pdbg(DBG_CRIT, "GPIO devices could not be correctly initialized\n");	    
	exit(err);
    }
    
    write(lcd_pipe, "Select mode:    Press left/right", 32);

    blink(0x0a, 125000); // left&right
    
    printf("Testing buttons... Press left/right and then center to exit.\n");
    fflush(NULL);
    
    while(keep)
    {
	pio_read_buttons(&but);
        switch(but) {
            case 0x02: // right
		write(lcd_pipe, "Pres center!    Dev mode ON     ", 32);
                sel_flag = 1;
		blink(0x01, 125000);
                break;

            case 0x08: // left 
		write(lcd_pipe, "Pres center!    Exec mode ON    ", 32);
                sel_flag = 2;
		blink(0x01, 125000);
                break;

            case 0x01:
                if(sel_flag){
                    pio_write_ledspos(0x00);
		    write(lcd_pipe, "Mode Selected  ", 16);
                    printf("Mode flag %d selected.\n",sel_flag);
                    exit(sel_flag);
                }
                break;
            default:
                break;
        }
	__msleep(10);
    }

    pio_write_ledspos(0x00);
    exit(-1);
}

void clean_exit()
{
    printf("\n...cleanup!\n");
    rt_task_delete(&ms_ptr);  
    pio_clean_all();
    close(lcd_pipe);
}

int main(int argc, char** argv)
{
    int err; 
    
    // Assure a clean exit
    signal(SIGTERM, clean_exit);	
    signal(SIGINT, clean_exit);	   
    
    // Avoid page faults	
    if( ( err = mlockall(MCL_CURRENT | MCL_FUTURE)) < 0 ) {
	printf("MAIN: Memory could not be locked. Exiting...\n");
	exit(-1);
    }   

    if( (err = rt_task_spawn(&ms_ptr, "Mode Selection Task", STACK_SIZE, MS_PRIO, 0, &main_task, NULL)) < 0){
	printf("MAIN: MS task could not be correctly initialized\n");
	exit(err);
    }	
	
    // wait for signal & return of signal handler
    pause();
    fflush(NULL);	

    return -1;
}
