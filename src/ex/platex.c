/** ******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: Example code for library testing

    License: Licensed under GPL2.0 

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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
// #include <sys/io.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

//Xenomai
#include <native/task.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <native/intr.h>

#include "gpio.h"            /* GPIO */
#include "platform_io.h"     /* High-level functions for GPIO devices */
#include "util.h"
#include "motors.h" 

#define STACK_SIZE 8192
#define WATCHDOG_PRIO 25
#define STD_PRIO 25

RT_TASK watchdog_ptr;
RT_TASK main_task_ptr;
RTIME watchdog_period_ns =  1000000000llu;
//RTIME watchdog_period_ns =  100000000llu;

int irq_counter = 0; 
int end = 0; 

RT_MUTEX mutex; 


void gpio_isr(void* cookie)
{	
    int err;
    RT_INTR* intr_desc = (RT_INTR*)cookie; 
    printf("IRQ: IRQ spawned. Waiting...\n");
    
    while(!end){
	if( ( err = rt_intr_wait(intr_desc, TM_INFINITE)) > 0){
	    if(++irq_counter == 10)
		    end = 1; //finish!
	}
	printf("IRQ: - IRQ counter %d\n",irq_counter);
	rt_intr_enable(intr_desc);
    }
}

// print out the interrupt-count periodically
void watchdog(void *cookie) {
	int err;
	unsigned but; 
/*	int k = 0xf8f; */
	int i = 0; 
	unsigned long overrun;
		
	if ((err = rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(watchdog_period_ns))) < 0) {
		util_pdbg(DBG_WARN, "WATCHDOG: - Error while set periodic, code %d\n",err);
		return;
	}
	
	while (!end) {
		err = rt_task_wait_period(&overrun);
		if (err) {
			util_pdbg(DBG_WARN,"WATCHDOG: Error while rt_task_wait_period %d\n",err);
			return;
		}
		if( (err = pio_read_buttons(&but))< 0)
			printf("WATCHDOG: error reading buttons\n");
		
		printf("WATCHDOG: - IRQ counter %d - Leds:0x%x But:0x%x\n",irq_counter,i,but);
//    		pio_write_ledspos(i);
/*		pio_read_ledspos(&but);		
		printf("readback:%d\n",but);*/
 		pio_write_leds4(i);

// 		pio_write_go_all(k<<i);
// 		pio_write_go_all(i);
		
		fflush(NULL);
		if(i == 0x1f)
		    i = 0;
		else
		    i++;
	}
}

void main_task(void* cookie)
{
	int err; 
	
    	util_pdbg(DBG_INFO, "Initializing GPIOs\n");
	
	if( (err = pio_init_all(gpio_isr, NULL)) < 0 ) {
		util_pdbg(DBG_CRIT, "GPIO devices could not be correctly initialized\n");	    
		exit(err);
	}
	
// 	for( i = 0xff80 ; i <= 0xffff ; i++ )
// 	    pio_write_go_all(i);
// 	
// 	while (!end);
}

// signal-handler, to ensure clean exit on Ctrl-C
void clean_exit(int dummy) {
	printf("cleanup\n");
	end = 1;
	pio_clean_all();
	rt_task_delete(&watchdog_ptr);
	rt_task_delete(&watchdog_ptr);
	exit(0);
}

int main( int argc, char** argv )
{
	int err; 
	util_pdbg(DBG_INFO, "MAIN: Starting...\n");		

	// Assure a clean exit
	signal(SIGTERM, clean_exit);	
	signal(SIGINT, clean_exit);	   
	
	// Avoid page faults	
	if( ( err = mlockall(MCL_CURRENT | MCL_FUTURE)) < 0 ) {
		util_pdbg(DBG_CRIT, "MAIN: Memory could not be locked. Exiting...\n");
		exit(-1);
	}

	print_banner();
	

	if( (err = rt_task_spawn(&main_task_ptr, "Main task", STACK_SIZE, STD_PRIO, 0, &main_task, NULL)) < 0){
		util_pdbg(DBG_CRIT, "MAIN: Main task could not be correctly initialized\n");
		exit(err);
	}

	if( (err = rt_task_spawn(&watchdog_ptr, "Watchdog", STACK_SIZE, WATCHDOG_PRIO, 0, &watchdog, NULL)) < 0){
		util_pdbg(DBG_CRIT, "MAIN: Watchdog could not be correctly initialized\n");
		exit(err);
	}
	    
	while(end==0);
	util_pdbg(DBG_INFO, "MAIN: Finished!\n");	
	return 0; 
}

