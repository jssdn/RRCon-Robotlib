/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: srf08.c Driver for SRF08 I2C Sonars

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

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

int srf08_get_echo(uint8_t address, int i2cbus, uint8_t n)
{
    uint8_t offset; 

    if( n >= 17 )
        return -EINVAL;

    offset = ( n << 1 ) + SRF08_REG_1STECHO_HIGH ; // First echo starts at 0x02 starts

    return ((int)i2cget(address, offset, i2cbus, 'b' ) << 8 ) | ((int) i2cget(address, offset + 1, i2cbus, 'b' )) ;
}

// TODO: BROADCAST RANGING COMMAND 
//       max range
//       analogue gain?
//       ANN

// TODO: MACROS AS FUNCTIONS

// inline 
// int srf08_fire_cm(uint8_t address, int i2cbus)
// {
//             return i2cset(I2C_SONAR0_ADDRESS, SRF08_REG_CMD,SRF08_CMD_RG_RESCM, I2C_SONAR0_BUS, 'b' );
// }
// 
// inline 
// int srf08_fire_inch(uint8_t address, int i2cbus)
// {
//             return i2cset(address, SRF08_REG_CMD,SRF08_CMD_RG_RESINCH, i2cbus, 'b' );
// }
// 
// inline 
// int srf08_fire_usec(uint8_t address, int i2cbus)
// {
//             return i2cset(address, SRF08_REG_CMD,SRF08_CMD_RG_USEC, i2cbus, 'b' );
// }
// 


