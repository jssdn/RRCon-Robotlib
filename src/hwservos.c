/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: hwservos.c Servo interface for hardware servo core controller
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

    NOTE: Done. Untested
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
#include <time.h>
//Xenomai
#include <native/mutex.h>
//--
#include "busio.h"
#include "util.h"
#include "hwservos.h"

int hwservos_init(HWSERVOS* servo, unsigned long base_add, unsigned long end_add, unsigned num_of)
{
    int i,err; 

    util_pdbg(DBG_INFO, "HWSERVOS: Initializing HWSERVOS:\n");

    // map
    if( (err = mapio_region(&(servo->vadd), base_add,end_add)) < 0 )
        return err; 

    servo->base_add = base_add;
    servo->end_add = end_add;
    
    if( num_of > HWSERVOS_MAX_NUM_OF )
	return -ECHRNG;

    servo->values = malloc(sizeof(unsigned)*num_of);
    for( i = 0; i < num_of ; i++ )
	servo->values[i] = 0;
    
    // Mutex init
    if( (err = rt_mutex_create(&(servo->mutex), NULL)) < 0 ){
	    util_pdbg(DBG_WARN, "HWSERVOS: Error rt_mutex_create: %d\n", err);
	    return err;
    } 
    //TODO: Error clean through tag cleaning
    return 0;
}

int hwservos_clean(HWSERVOS* servo)
{
    int err,i;

    util_pdbg(DBG_DEBG, "HWSERVOS: Cleaning HWSERVOS...\n");
    
    //Disable servos before unmapping
    for( i = 0 ; i <= servo->num_of ; i++ )
	hwservos_disable(servo,i);

    if ( (err = unmapio_region(&(servo->vadd), servo->base_add, servo->end_add)) < 0 ){
	util_pdbg(DBG_WARN, "HWSERVOS: couldn't be unmapped at virtual= 0x%x . Error : %d \n", &(servo->vadd), err);
	return err; 
    }
    
    // delete mutex
    if( ( err = rt_mutex_delete(&(servo->mutex)) ) < 0 ){
	util_pdbg(DBG_WARN, "HWSERVOS: Mutex cannot be deleted \n");
	return err; 
    }    
    
    return 0;    
}

int hwservos_set_pos(HWSERVOS* servo, unsigned num, unsigned value)
{
    int err; 

    // Sanity check
    if(value < HWSERVOS_TIME_MIN_ANGLE ) 
	value = HWSERVOS_TIME_MIN_ANGLE ; 
    else if (value > HWSERVOS_TIME_MAX_ANGLE ) 
	    value = HWSERVOS_TIME_MAX_ANGLE ; 
    
    if( (err = rt_mutex_acquire(&(servo->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
 
    *(servo->vadd + num) = value | HWSERVOS_EN_MASK ; 
    servo->values[num] =value;
    
    if( (err = rt_mutex_release(&(servo->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }

    return 0;
}

/* NOTE: Returned value IS NOT BASED IN HW feedback, but in latched values */
int hwservos_get_pos(HWSERVOS* servo, unsigned num, unsigned* ret)
{
    int err; 
   
    if( (err = rt_mutex_acquire(&(servo->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
 
    *ret = *(servo->vadd + num) & ~(HWSERVOS_EN_MASK) ; // Remove the enable mask

    if( (err = rt_mutex_release(&(servo->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }

    return 0;
}

int hwservos_enable(HWSERVOS* servo, unsigned num)
{    
    int err;
    
    if( (err = rt_mutex_acquire(&(servo->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
 
    *(servo->vadd + num) |= HWSERVOS_EN_MASK ;

    if( (err = rt_mutex_release(&(servo->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }

    return 0;
}

int hwservos_disable(HWSERVOS* servo, unsigned num)
{
    int err; 
    
    if( (err = rt_mutex_acquire(&(servo->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
 
    *(servo->vadd + num) &= ~(HWSERVOS_EN_MASK) ;

    if( (err = rt_mutex_release(&(servo->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "HWSERVOS: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }

    return 0;
}
