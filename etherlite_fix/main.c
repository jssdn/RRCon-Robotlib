/** *******************************************************************************
*
*    Project: Robotics library for the Autonomous Robotics Development Platform 
*    Port by: Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
*    
*    NOTE: This code is based on the Xilinx XAPP1042.
* 	 
* 	Description:
* 		An example application which accesses PHY registers:
* 		- PHY ID registers are displayed
* 		- 1000MB Autonegotiation is disabled
* 		- The results of autonegotiation are displayed
*
*     Xilinx disclaimer below apply.
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*     SOLELY FOR USE IN DEVELOPING PROGRAMS AND SOLUTIONS FOR
*     XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION
*     AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION
*     OR STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS
*     IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*     FOR A PARTICULAR PURPOSE.
*
*     (c) Copyright 2008 Xilinx, Inc.
*     All rights reserved.
*
******************************************************************************* **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
// Xenomai - Robotlib GPIO needs to be launched from an RT enviroment
#include <native/task.h>
// -- 

#include "mii-bitbang.h"
#include "marvell_88e1111.h"

/*
 * The MII address of the PHY on the board this software will run on
 * ML403
 */
#define PHY_ADDR   0
#define PHY_IDO_REG  2
#define PHY_ID1_REG  3

/*
 * GPIO Connected to PHY MDC/MDIO pins.
 */
GPIO mii_gpio;

/*
 * Xenomai variables for robotlib compatibility 
 */

#define STACK_SIZE 8192
#define ETH_PRIO 2
RT_TASK main_task_ptr;

/* RT Main */
void main_task(void* cookie) 
{
        
   uint16_t val, phy_id;

   printf("\n\rAccessing PHY with GPIO:\n\r");

    //TODO:
   /*
    * Initialize the GPIO
    */
    if ( MiiGpio_Init(&mii_gpio) < 0 ){
	perror(NULL);	
	printf("\n Couldn't init GPIO device. Exiting...:\n");
	fflush(NULL);
	exit(-1); 
    }
	

   /*
    * Set the PHY access functions to the GPIO Bitbang functions.
    */
   marvell_phy_setvectors((PhyRead_t*)MiiGpio_PhyRead,
                          (PhyWrite_t*)MiiGpio_PhyWrite);

   phy_id = PHY_ADDR;
   printf("Read from phy %d\n\r", phy_id);
   MiiGpio_PhyRead(&mii_gpio, phy_id, PHY_IDO_REG, &val);
   printf("REG %d: 0x%.4x\n\r", PHY_IDO_REG, val);
   MiiGpio_PhyRead(&mii_gpio, phy_id, PHY_ID1_REG, &val);
   printf("REG %d: 0x%.4x\n\r", PHY_ID1_REG, val);

   /*
    * Verify that a Marvell PHY is present
    */
   val = marvell_phy_detected(&mii_gpio, PHY_ADDR);
   if (val) { 
       printf("Marvell PHY not detected.\n\r");
       exit(-1);
   }

   /*
    * Wait for autonegotiation to complete.
    */
   printf("Waiting for auto-negotiation to complete.\n\r");
   do {
       val = marvell_phy_link_status(&mii_gpio, PHY_ADDR);
   } while (val);

   /*
    * Display present results of auto-negotiation.
    */
   marvell_phy_display_status(&mii_gpio, PHY_ADDR);

   printf("Disabling 1000MB and initiating re-negotiation.\n\r");
   marvell_phy_set_1000mb_autoneg(&mii_gpio, PHY_ADDR, -1);
   
   /*
    * Wait for autonegotiation to complete.
    */
   printf("Waiting for auto-negotiation to complete.\n\r");
   
   do {
       val = marvell_phy_link_status(&mii_gpio, PHY_ADDR);
   } while (val);
 
   /*
    * Display new results of auto-negotiation.
    */
   marvell_phy_display_status(&mii_gpio, PHY_ADDR);
}

/* RT task to clean objects ( some cannot be cleaned from a non-rt space ) */
void clean_exit()
{
    //gpios
    //task
//     rt_task_delete(&hwservos_ptr);    
}

int main(void)
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

    if( (err = rt_task_spawn(&main_task_ptr, "Main task", STACK_SIZE, ETH_PRIO, 0, &main_task, NULL)) < 0){
	    printf("MAIN: Main task could not be correctly initialized\n");
	    exit(err);
    }
    
    // wait for signal & return of signal handler
    pause();
    fflush(NULL);
    
    return 0; 
}
