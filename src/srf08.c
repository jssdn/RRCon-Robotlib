/**
    @file srf08.c 
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Driver for SRF08 I2C Sonars
    
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

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>

#include "srf08.h"
#include "i2ctools.h"
#include "util.h"

/**
* @brief Initialization of the SRF08 sonar device
*
* @param sonar SRFO8 sonar peripheral to init
* @param i2c I2C Bus where the SRF08 is attached
* @return 0 on success. Otherwise error. 
*
* Initizializes the structure and creates corresponding mutex 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int srf08_init(SRF08* sonar,I2CDEV* i2c, uint8_t address)
{
    int err; 

    util_pdbg(DBG_INFO, "Initializing SRF08 sonar...\n");
    
    if( i2c == NULL || sonar == NULL ){	
	util_pdbg(DBG_WARN, "SRF08: Cannot use non-initialized devices...\n");
	return -EFAULT; 
    }
    
    sonar->i2c = i2c; 
    sonar->address = address; 

    UTIL_MUTEX_CREATE("SRF08",&(sonar->mutex), NULL);

    return 0;     
}

/**
* @brief Clean for the SRF08 sonar device
*
* @param sonar SRFO8 sonar peripheral to init
* @return 0 on success. Otherwise error. 
*
* Cleans the structure and creates corresponding mutex 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int srf08_clean(SRF08* sonar)
{
    int err; 

    util_pdbg(DBG_INFO, "Cleaning the SRF08...\n");
    
    sonar->i2c = NULL;
    sonar->address = 0x00;    
    
    UTIL_MUTEX_DELETE("SRF08", &(sonar->mutex));
    
    return err;     
}

/**
* @brief Read one echo from the the sonar
*
* @param sonar SRFO8 sonar peripheral to init
* @param n Echo number ( 0 - 17 ) 
* @return read value. Negative values (int) should be considered as errors.  
*
* Get one echo from the sonar readings 
* @note This function requires that the sensor has previously been shooted
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int srf08_get_echo(SRF08* sonar, uint8_t n)
{
    int err,res; 

    uint8_t offset; 

    if( n >= 17 )
        return -EINVAL;

    offset = ( n << 1 ) + SRF08_REG_1STECHO_HIGH ; // First echo starts at 0x02 starts
    
    UTIL_MUTEX_ACQUIRE("SRF08",&(sonar->mutex),TM_INFINITE);
    
    //Get 16 bits TODO: why not just get word!?        
    res = (((uint16_t)(i2c_get(sonar->i2c, sonar->address, offset, 'b' ))) << 8 ) | ((uint16_t) i2c_get(sonar->i2c, sonar->address, offset + 1, 'b' ));
    
    UTIL_MUTEX_RELEASE("SRF08",&(sonar->mutex));

    return res; 
}

/**
* @brief Read the light sensor from the SRF08
*
* @param sonar SRFO8 sonar peripheral to init
* @return read value ( 8 bit payload ). Negative values (int) should be considered as errors.  
*
* Get the LDR value from the SRF08 readings 
* @note This function requires that the sensor has previously been shooted
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int srf08_get_light(SRF08* sonar) 
{
    int err,res; 
    
    UTIL_MUTEX_ACQUIRE("SRF08",&(sonar->mutex),TM_INFINITE);

    res = i2c_get(sonar->i2c, sonar->address,  'b' , SRF08_REG_LIGHT);
    
    UTIL_MUTEX_RELEASE("SRF08",&(sonar->mutex));
    
    return res; 
}

/**
* @brief Shoots a sonar pulse
*
* @param sonar SRFO8 sonar peripheral to init
* @param daddress Command ( usecs, cm,. inch )
* @return 0 on success. Negative values (int) should be considered as errors.  
*
* Fires a sonar pulse
* @note This function requires its corresponding wait time afterwards in order to be able to read correct values
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

static int srf08_fire(SRF08* sonar, uint8_t daddress, uint8_t cmd)
{
    int err,err2; 
    
    UTIL_MUTEX_ACQUIRE("SRF08",&(sonar->mutex),TM_INFINITE);
    
    err2 = i2c_set(sonar->i2c, sonar->address, daddress, 'b', cmd );
    
    UTIL_MUTEX_RELEASE("SRF08",&(sonar->mutex));
    
    return err2; 
}

/**
* @brief Shoots a sonar pulse and tell the SRFO8 to calculate distances in inches
*
* @param sonar SRFO8 sonar peripheral to init
* @return 0 on success. Negative values (int) should be considered as errors.  
*
* Fires a sonar pulse
* @note This function requires its corresponding wait time afterwards in order to be able to read correct values
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int srf08_fire_inch(SRF08* sonar)
{
    return srf08_fire(sonar, SRF08_REG_CMD,SRF08_CMD_RG_RESINCH);
}

/**
* @brief Shoots a sonar pulse and tell the SRFO8 to calculate distances in cm 
*
* @param sonar SRFO8 sonar peripheral to init
* @return 0 on success. Negative values (int) should be considered as errors.  
*
* Fires a sonar pulse
* @note This function requires its corresponding wait time afterwards in order to be able to read correct values
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int srf08_fire_cm(SRF08* sonar)
{
    return srf08_fire(sonar, SRF08_REG_CMD,SRF08_CMD_RG_RESCM);
}

/**
* @brief Shoots a sonar pulse and tell the SRFO8 to calculate distances in usec ( time of flight ) 
*
* @param sonar SRFO8 sonar peripheral to init
* @return 0 on success. Negative values (int) should be considered as errors.  
*
* Fires a sonar pulse
* @note This function requires its corresponding wait time afterwards in order to be able to read correct values
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int srf08_fire_usec(SRF08* sonar)
{
    return srf08_fire(sonar, SRF08_REG_CMD,SRF08_CMD_RG_RESUSEC);
}

/* TODO: Needed? */
// int srf08_get_fw( address, i2cbus)
// {
//     i2c_get(address, SRF08_REG_CMD, i2cbus, 'b' );
// }


// TODO: BROADCAST RANGING COMMAND 
//       max range
//       analogue gain?
//       ANN

