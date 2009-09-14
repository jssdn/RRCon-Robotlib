/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: tcn75.c Driver for I2C Temperature Sensor Microchip TCN75

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

// TCN75
// TODO: Implement TSET/THYST

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>

#include "tcn75.h"
#include "i2ctools.h"

inline int tcn75_init( uint8_t address, int i2cbus)
{
    // set no interrupt-> not wired
    return i2cset(address, TCN75_REG_CONFIG, TCN75_REG_CONFIG_ON, i2cbus, 'b' );
}

//faster, only needs one access.Only 8 bit resolution( instead of 9)
inline int tcn75_read_msb(uint8_t address, int i2cbus,int8_t* temp)
{
    int res;
    if ( (res = i2cget( address, TCN75_REG_TEMP, i2cbus, 'b' )) < 0 )
        return res; //error reading
    *temp = res; 
    return 0;
}

int tcn75_read(uint8_t address, int i2cbus,int16_t* temp)
{
    int res;  
    if ( (res = i2cget( address, TCN75_REG_TEMP, i2cbus, 'w' )) < 0 )
            return res; //error reading

    *temp = (res << 1) + ((res & 0x8000)? 1 : 0);

        //TODO: improve this 

    if(( res & 0x0080 )) // negative
            *temp = 0 - *temp; 

    return 0;
}

inline int tcn75_shutdown( uint8_t address, int i2cbus)
{
    return i2cset(address, TCN75_REG_CONFIG, TCN75_REG_CONFIG_SHUTDOWN_MASK, i2cbus, 'b' );
}

