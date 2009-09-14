/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: HMC6352 magnetic compass driver 

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

// HMC6352 magnetic compass driver 
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>

#include "hmc6352.h"
#include "i2ctools.h"

int hmc6532_idcheck(uint8_t address, int i2cbus)
{
    int res; 
    if( (res = i2cget_3com(address, 
                           HMC6352_CMD_READ_EEPROM,
                           HMC6352_EE_REG_ADDRESS,
                           i2cbus)) < 0 )
        return res; 

    return  (uint8_t)res == HMC6352_ID? 0 : -ENODEV ; 
}

int hmc6532_init_standby(uint8_t address, int i2cbus)
{
    int res; 
    if( (res = hmc6532_idcheck(address,i2cbus)) < 0 )
        return res; 

    return i2cset(address,
                 HMC6352_CMD_WRITE_RAM,
                 (HMC6352_RAM_REG_OPMODE << 8) | HMC6352_REG_OPMODE_OP_STANDBY,
                 i2cbus, 
                 'w' );
}


int hmc6532_init_query(uint8_t address, int i2cbus)
{
    int res; 
    if( (res = hmc6532_idcheck(address,i2cbus)) < 0 )
        return res; 

    return i2cset(address, 
                  HMC6352_CMD_WRITE_RAM,
                  (HMC6352_RAM_REG_OPMODE << 8) | HMC6352_REG_OPMODE_OP_QUERY,
                  i2cbus, 
                  'w' );
}

int hmc6532_init_continous(uint8_t address, int i2cbus,uint8_t freq)
{
    int res;
    uint16_t com; 

    if( (res = hmc6532_idcheck(address,i2cbus)) < 0 )
        return res; 
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

    // TODO: SET FREQ
    return i2cset(address, 
                  HMC6352_CMD_WRITE_RAM,
//                  (HMC6352_RAM_REG_OPMODE << 8) | HMC6352_REG_OPMODE_FREQ_10HZ | HMC6352_REG_OPMODE_OP_CONTINOUS,
                  com,
                  i2cbus, 
                  'w' );
}

// TODO: READ functions are not well made for the operating modes
// standby -> should send two reads
// query -> one read,gets the previos
// continous -> does need the 'A' command
int hmc6532_read_nowait(uint8_t address, int i2cbus, uint16_t* degrees)
{
    int res; 

    if (( res =  i2cget(address, HMC6352_CMD_GETDATA, i2cbus, 'w')) < 0 ) 
        return res; 

    res &= 0xffff; 
    *degrees = ( res << 8 ) | ( res >> 8 );
    return 0; 
}

inline int hmc6532_read_wait(uint8_t address, int i2cbus, uint16_t* degrees)
{
     usleep(6000); // we need to wait at least this 
     return hmc6532_read_nowait(address, i2cbus, degrees); 
}

inline int hmc6532_enter_calibration(uint8_t address, int i2cbus)
{
    return i2cset_1com( address, HMC6352_CMD_ENTER_CALIB, i2cbus);
}

inline int hmc6532_exit_calibration(uint8_t address, int i2cbus)
{
    int res; 
    if((res =  i2cset_1com( address, HMC6352_CMD_EXIT_CALIB, i2cbus) ) < 0 ) 
        return res;
    usleep(14000); // Minimum delay needed
    return 0;
}

inline int hmc6532_sleep(uint8_t address, int i2cbus)
{
    return i2cset_1com( address, HMC6352_CMD_SLEEP, i2cbus);
}

inline int hmc6532_wakeup(uint8_t address, int i2cbus)
{
    int res;
    if((res =  i2cset_1com( address, HMC6352_CMD_WAKEUP, i2cbus) ) < 0 ) 
        return res;

    usleep(100); // minimum delay needed 
    return 0;
}

// int hmc6532_get_heading(uint8_t address, int i2cbus, int16_t* degrees);
// 
// int hmc6532_get_xy_raw(uint8_t address, int i2cbus, int16_t* x, int16_t* x );
// 
// int hmc6532_get_xy(uint8_t address, int i2cbus, int16_t* x, int16_t* x );
// 
// int hmc6532_saveop_to_eeprom(uint8_t address, int i2cbus);
