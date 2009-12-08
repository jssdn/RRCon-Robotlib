/**
    @file hwservos.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Servo interface for hardware RC servo core controller
    
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
#include <time.h>
//Xenomai
#include <native/mutex.h>
//--
#include "busio.h"
#include "util.h"
#include "hwservos.h"

/**
* @brief Initialization for Servo IP Core device data structure. 
*
* @param servo Servo IP core peripheral to init
* @param base_add Physical base IO address of peripheral
* @param end_add Physical final IO address of peripheral
* @param num_of Number of active servos in Core
* @return 0 on success. Otherwise error. 
*
* Maps IO region, set flags and init mutex.
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

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
    
    servo->num_of = num_of; 

    servo->values = malloc(sizeof(unsigned)*num_of);
    
    for( i = 0; i < num_of ; i++ )
	servo->values[i] = 0;
    
    UTIL_MUTEX_CREATE("HWSERVOS",&(servo->mutex),NULL);

    //TODO: Error clean through tag cleaning
    return 0;
}

/**
* @brief Clean the Servo IP Core device data structure. 
*
* @param servo Servo IP core peripheral to init
* @return 0 on success. Otherwise error. 
*
* Unmaps IO region, and clean mutex.
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int hwservos_clean(HWSERVOS* servo)
{
    int err,i;

    util_pdbg(DBG_DEBG, "HWSERVOS: Cleaning HWSERVOS...\n");
    
    //Disable servos before unmapping
    for( i = 0 ; i < servo->num_of ; i++ )
    {
	util_pdbg(DBG_DEBG, "\t-> HWSERVOS: Disabling servo %d\n", i);
	hwservos_disable(servo,i);
    }
    
    if ( (err = unmapio_region(&(servo->vadd), servo->base_add, servo->end_add)) < 0 ){
	util_pdbg(DBG_WARN, "\t-> HWSERVOS: couldn't be unmapped at virtual= 0x%x . Error : %d \n", &(servo->vadd), err);
	return err; 
    }
    
    free(servo->values);

    UTIL_MUTEX_DELETE("\t-> HWSERVOS",&(servo->mutex));

    return 0;    
}

/**
* @brief Sets control to a certain angle
*
* @param servo Servo IP core peripheral to init
* @param num Servo to interact with
* @param value Time in us that will be applied to the servos
* @return 0 on success. Otherwise error. 
*
* Control stays active if no other order is performed. 
*
* @note There is a hard and a soft sanity check over the value. If time in us is over or under the thresholds the servo can be 
* 	physically damaged. 
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/


int hwservos_set_pos(HWSERVOS* servo, unsigned num, unsigned value)
{
    int err; 

    // Sanity check
    if(value < HWSERVOS_TIME_MIN_ANGLE ) 
	value = HWSERVOS_TIME_MIN_ANGLE ; 
    else if (value > HWSERVOS_TIME_MAX_ANGLE ) 
	    value = HWSERVOS_TIME_MAX_ANGLE ; 
    
    UTIL_MUTEX_ACQUIRE("HWSERVOS",&(servo->mutex),TM_INFINITE);
 
    *(servo->vadd + num) = value | HWSERVOS_EN_MASK ; 
    servo->values[num] =value;
    
    UTIL_MUTEX_RELEASE("HWSERVOS",&(servo->mutex));
    
    return 0;
}

/**
* @brief Gets the set point for the Servo
*
* @param servo Servo IP core peripheral to init
* @param num Servo to interact with
* @param ret Time in us that is being applied to the servos
* @return 0 on success. Otherwise error. 
*
* @note Returned value IS NOT BASED IN HW feedback, but in latched values
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hwservos_get_pos(HWSERVOS* servo, unsigned num, unsigned* ret)
{
    int err; 
   
    UTIL_MUTEX_ACQUIRE("HWSERVOS",&(servo->mutex),TM_INFINITE);
 
    *ret = *(servo->vadd + num) & ~(HWSERVOS_EN_MASK) ; // Remove the enable mask

    UTIL_MUTEX_RELEASE("HWSERVOS",&(servo->mutex));
    
    return 0;
}

/**
* @brief Enable active the control independently
*
* @param servo Servo IP core peripheral to init
* @param num Servo to interact with
* @return 0 on success. Otherwise error. 
*
* Control stays active if no other order is performed. By enabling it, you force the system to find the set point continously.
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hwservos_enable(HWSERVOS* servo, unsigned num)
{    
    int err;
    
    UTIL_MUTEX_ACQUIRE("HWSERVOS",&(servo->mutex),TM_INFINITE);
    
    *(servo->vadd + num) |= HWSERVOS_EN_MASK ;

    UTIL_MUTEX_RELEASE("HWSERVOS",&(servo->mutex));
    
    return 0;
}

/**
* @brief Disable active the control independently
*
* @param servo Servo IP core peripheral to init
* @param num Servo to interact with
* @return 0 on success. Otherwise error. 
*
* Control stays active if no other order is performed. By disabling it, no internal force will be applied to the servo to find 
* the set point.
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hwservos_disable(HWSERVOS* servo, unsigned num)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("HWSERVOS",&(servo->mutex),TM_INFINITE);
    
    *(servo->vadd + num) &= ~(HWSERVOS_EN_MASK) ;

    UTIL_MUTEX_RELEASE("HWSERVOS",&(servo->mutex));

    return 0;
}
