/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: srf08.c Driver for SRF08 I2C Sonars

    License: Licensed under GPL2.0 
    
    Copyright (C) Jorge Sánchez de Nova
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

int srf08_clean(SRF08* sonar)
{
    int err; 

    util_pdbg(DBG_INFO, "Cleaning the SRF08...\n");
    
    sonar->i2c = NULL;
    sonar->address = 0x00;    
    
    UTIL_MUTEX_DELETE("SRF08", &(sonar->mutex));
    
    return err;     
}

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

int srf08_get_light(SRF08* sonar) 
{
    int err,res; 
    
    UTIL_MUTEX_ACQUIRE("SRF08",&(sonar->mutex),TM_INFINITE);

    res = i2c_get(sonar->i2c, sonar->address,  'b' , SRF08_REG_LIGHT);
    
    UTIL_MUTEX_RELEASE("SRF08",&(sonar->mutex));
    
    return res; 
}


static int srf08_fire(SRF08* sonar, uint8_t daddress, uint8_t cmd)
{
    int err,err2; 
    
    UTIL_MUTEX_ACQUIRE("SRF08",&(sonar->mutex),TM_INFINITE);
    
    err2 = i2c_set(sonar->i2c, sonar->address, daddress, 'b', cmd );
    
    UTIL_MUTEX_RELEASE("SRF08",&(sonar->mutex));
    
    return err2; 
}

inline int srf08_fire_inch(SRF08* sonar)
{
    return srf08_fire(sonar, SRF08_REG_CMD,SRF08_CMD_RG_RESINCH);
}

inline int srf08_fire_cm(SRF08* sonar)
{
    return srf08_fire(sonar, SRF08_REG_CMD,SRF08_CMD_RG_RESCM);
}

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

