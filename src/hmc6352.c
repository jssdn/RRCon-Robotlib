/**
    @file hmc6352.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief HMC6352 Magnetic compass driver
    
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
    
    @note Read the HMC6352 datasheet for a better understanding of the functions 
    @warning Needs signicant improvements. Check TODOs!
    
 */

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>
// Xenomai
#include <native/mutex.h>
//--
#include "hmc6352.h"
#include "i2ctools.h"
#include "util.h"

/**
* @brief Initialization for HMC6352 data structure. 
*
* @param compass HMC6352 device to init
* @param i2c I2C buss where the HMC6352 is attached
* @param address I2C address
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int hmc6532_init(HMC6352* compass, I2CDEV* i2c, uint8_t address)
{
    int err; 

    util_pdbg(DBG_INFO, "Initializing HMC6352 compass...\n");
    
    if( i2c == NULL || compass == NULL ){	
	util_pdbg(DBG_WARN, "HMC6352: Cannot use non-initialized devices...\n");
	return -EFAULT; 
    }
    
    compass->i2c = i2c; 
    compass->address = address; 

    UTIL_MUTEX_CREATE("HMC6352",&(compass->mutex), NULL);

    return 0; 
}

/**
* @brief Cleans for HMC6352 data structure. 
*
* @param compass HMC6352 device to clean
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int hmc6532_clean(HMC6352* compass)
{
    int err; 

    util_pdbg(DBG_INFO, "Cleaning HMC6352 compass...\n");
    
    compass->i2c = NULL;
    compass->address = 0x00; 
    
    UTIL_MUTEX_DELETE("HMC6352", &(compass->mutex));
    
    return 0; 
}

/**
* @brief Checks that is the HMC6352 compass what it is attached in the i2c bus
*
* @param compass HMC6352 device
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_idcheck(HMC6352* compass)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);
    
    err = i2c_get_3com(compass->i2c, compass->address, HMC6352_CMD_READ_EEPROM,HMC6352_EE_REG_ADDRESS);
    
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    if(err < 0) 
	return err; 
    
    return  (uint8_t)err == HMC6352_ID? 0 : -ENODEV ; 
}

/**
* @brief Set the HMC6352 compass in stand-by mode
*
* @param compass HMC6352 device
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_init_standby(HMC6352* compass)
{
    int err; 
    
    if( (err = hmc6532_idcheck(compass)) < 0 )
        return err; 

    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);
    
    err = i2c_set(compass->i2c,compass->address,HMC6352_CMD_WRITE_RAM,
		 'w', (HMC6352_RAM_REG_OPMODE << 8) | HMC6352_REG_OPMODE_OP_STANDBY);
		 
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    return err; 
}

/**
* @brief Set the HMC6352 compass in query mode
*
* @param compass HMC6352 device
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_init_query(HMC6352* compass)
{
    int err; 
    
    if( (err = hmc6532_idcheck(compass)) < 0 )
        return err; 

    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);
    
    err = i2c_set(compass->i2c, compass->address,HMC6352_CMD_WRITE_RAM,
                  'w', (HMC6352_RAM_REG_OPMODE << 8) | HMC6352_REG_OPMODE_OP_QUERY);
		  
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    return err; 
}

/**
* @brief Set the HMC6352 compass in continous mode
*
* @param compass HMC6352 device
* @param freq Update frequency
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_init_continous(HMC6352* compass, uint8_t freq)
{
    int err;
    uint16_t com; 

    if( (err = hmc6532_idcheck(compass)) < 0 )
        return err; 
    
    com = (HMC6352_RAM_REG_OPMODE << 8) | HMC6352_REG_OPMODE_OP_CONTINOUS; 

    switch(freq)
    {
        case 1: 
            com |= HMC6352_REG_OPMODE_FREQ_1HZ; 
            break; 
        case 5:
            com |= HMC6352_REG_OPMODE_FREQ_5HZ; 
            break; 
        case 10: 
            com |= HMC6352_REG_OPMODE_FREQ_10HZ; 
            break; 
        case 20:
            com |= HMC6352_REG_OPMODE_FREQ_20HZ; 
            break; 
        default:
            com |= HMC6352_REG_OPMODE_FREQ_1HZ; //default 1 Hz
            break; 
    }

    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);

    // TODO: SET FREQ
    err = i2c_set(compass->i2c,compass->address,HMC6352_CMD_WRITE_RAM,
		   'w' ,com);
//                  (HMC6352_RAM_REG_OPMODE << 8) | HMC6352_REG_OPMODE_FREQ_10HZ | HMC6352_REG_OPMODE_OP_CONTINOUS);

		  
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    return err; 
}

/**
* @brief Reads the measurement in degrees without waiting( last measurement ): 
*
* @param compass HMC6352 device
* @param degrees Measured heading in degrees. 
* @return 0 on success. Otherwise error. 
*
* @warning Needs improvements!
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_read_nowait(HMC6352* compass, uint16_t* degrees)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);

    // TODO: READ functions are not well made for the operating modes
    // standby -> should send two reads
    // query -> one read,gets the previos
    // continous -> does need the 'A' command
    err =  i2c_get(compass->i2c, compass->address, HMC6352_CMD_GETDATA, 'w'); 
    
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));

    if( err < 0 )
	return err; 
    
    err &= 0xffff; 
    *degrees = ( err << 8 ) | ( err >> 8 );
    
    return 0; 
}

/**
* @brief Reads the measurement in degrees
*
* @param compass HMC6352 device
* @param degrees Measured heading in degrees. 
* @return 0 on success. Otherwise error. 
*
* @warning This functions put the task to sleep until the new measurement is acquired 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int hmc6532_read_wait(HMC6352* compass, uint16_t* degrees)
{
     __usleep(6000); // we need to wait at least this 
     return hmc6532_read_nowait(compass, degrees); 
}

/**
* @brief Sets the HMC6352 in calibration mode
*
* @param compass HMC6352 device
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_enter_calibration(HMC6352* compass)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);   
    
    err = i2c_set_1com( compass->i2c, compass->address, HMC6352_CMD_ENTER_CALIB);
    
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    return err; 
}

/**
* @brief Exits calibration mode
*
* @param compass HMC6352 device
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_exit_calibration(HMC6352* compass)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);   
    
    err =  i2c_set_1com( compass->i2c, compass->address, HMC6352_CMD_EXIT_CALIB);        
    
    __usleep(14000); // Minimum delay needed
    
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    if(err < 0 ) 
	return err; 
    
    return 0;
}

/**
* @brief Sets the HMC6352 in sleep mode
*
* @param compass HMC6352 device
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_sleep(HMC6352* compass)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);   
    
    err = i2c_set_1com( compass->i2c, compass->address, HMC6352_CMD_SLEEP);
    
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    return err; 
}

/**
* @brief Wakes up the HMC6352 when in sleep mode
*
* @param compass HMC6352 device
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int hmc6532_wakeup(HMC6352* compass)
{
    int err;
    
    UTIL_MUTEX_ACQUIRE("HMC6352",&(compass->mutex),TM_INFINITE);   
    
    err =  i2c_set_1com( compass->i2c, compass->address, HMC6352_CMD_WAKEUP); 
    
    __usleep(100); // minimum delay needed 
    
    UTIL_MUTEX_RELEASE("HMC6352",&(compass->mutex));
    
    if ( err < 0 ) 
	return err; 
    
    return 0;
}


// TODO
// int hmc6532_get_heading(uint8_t address, int i2cbus, int16_t* degrees);
// 
// int hmc6532_get_xy_raw(uint8_t address, int i2cbus, int16_t* x, int16_t* x );
// 
// int hmc6532_get_xy(uint8_t address, int i2cbus, int16_t* x, int16_t* x );
// 
// int hmc6532_saveop_to_eeprom(uint8_t address, int i2cbus);
