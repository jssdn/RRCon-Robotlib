/**
    @file lis3lv02dl.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Driver for I2C LIS3LV02DL Accelerometer
    
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
    
    @note See application note AN2381 ST 

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
//Xenomai
#include <native/mutex.h>
//--
#include "lis3lv02dl.h"
#include "i2ctools.h"
#include "busio.h"
#include "util.h"

/**
* @brief Initialization for the LIS3LV02DL device
*
* @param acc LIS3LV02DL accelerometer to init
* @param i2c I2C buss where the LIS3LV02DL is attached
* @param address I2C address
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int lis3lv02dl_init(LIS3LV02DL* acc, I2CDEV* i2c, uint8_t address)
{
    int err; 

    util_pdbg(DBG_INFO, "Initializing LIS3LV02DL accelerometer...\n");
    
    if( i2c == NULL || acc == NULL ){	
	util_pdbg(DBG_WARN, "LIS3LV02DL: Cannot use non-initialized devices...\n");
	return -EFAULT; 
    }
    
    acc->i2c = i2c; 
    acc->address = address; 

    UTIL_MUTEX_CREATE("LIS3LV02DL",&(acc->mutex), NULL);

    return 0; 
}

/**
* @brief Clean for the LIS3LV02DL device
*
* @param acc LIS3LV02DL accelerometer to clean
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int lis3lv02dl_clean(LIS3LV02DL* acc)
{
    int err; 

    util_pdbg(DBG_INFO, "Cleaning the LIS3LV02DL accelerometer...\n");
    
    err = lis3lv02dl_poweroff(acc); 
    
    acc->i2c = NULL;
    acc->address = 0x00; 
    
    UTIL_MUTEX_DELETE("LIS3LV02DL", &(acc->mutex));
    
    return err; 
}

/**
* @brief ID check for the LIS3LV02DL
*
* @param acc LIS3LV02DL accelerometer
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int lis3lv02dl_id_check(LIS3LV02DL* acc)
{
    int err,res; 
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);
        
    res = i2c_get(acc->i2c, acc->address, LIS3_REG_WHOAMI, 'b' );
    
    UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));

    return res == LIS3_ID? 0 : -1; 
}


/**
* @brief Turns the LIS3LV02DL off for power saving
*
* @param acc LIS3LV02DL accelerometer
* @return 0 on success. Otherwise error. 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int lis3lv02dl_poweroff(LIS3LV02DL* acc)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);

    err = i2c_set(acc->i2c, acc->address, LIS3_REG_CTRLREG1, 'b', LIS3_REG_CTRLREG1_PD_OFF );
    
    UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
    
    return err; 
}

/**
* @brief Samples the instantaneous acceleration data from the LIS3LV02DL 
*
* @param acc LIS3LV02DL accelerometer
* @return 0 on success. Otherwise error. 
*
* Data is stored into the LIS3LV02DL data structure
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int lis3lv02dl_read(LIS3LV02DL* acc)
{
    int err,i; 
    uint16_t tmp[6];
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);

    if ( ( err = i2c_get(acc->i2c, acc->address, LIS3_REG_STATUSREG, 'b' ) ) < 0 ) {
        util_pdbg(DBG_WARN, "LIS3LV02DL: Cannot read 0x%x from LIS3LV02DL\n", LIS3_REG_STATUSREG ) ; 
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
        return err; 
    }

//     if( !(err & LIS3_REG_STATUSREG_ZYXDA_MASK) ){
// 	util_pdbg(DBG_WARN, "LIS3LV02DL: No new data available\n" ) ; 
// 	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
//         return -EAGAIN; // No new data available
//     }
    
    if( (err & LIS3_REG_STATUSREG_ZYXOR_MASK) )
        acc->data_overrun = 1;
    else 
        acc->data_overrun = 0;

   //TODO:more efficient by reading all the bytes continously 
   for( i = 0 ; i < 6 ; i++ )
   {
//         if(( tmp[i] = i2c_get(address, LIS3_REG_OUTX_L + i , i2cbus, 'b' )) < 0) 
//             return tmp[i]; //i2c error
        tmp[i] = i2c_get(acc->i2c, acc->address, LIS3_REG_OUTX_L + i , 'b' );

   }

    // remove lower 4 bits ( noise ) 
    acc->xacc = (tmp[0] & 0xf0)| (tmp[1] << 8);     
    acc->yacc = (tmp[2] & 0xf0)| (tmp[3] << 8);
    acc->zacc = (tmp[4] & 0xf0)| (tmp[5] << 8);
    
    UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
    
    return 0;
}

/**
* @brief Samples the instantaneous acceleration data and set the calibration parameters for the LIS3LV02DL 
*
* @param acc LIS3LV02DL accelerometer
* @return 0 on success. Otherwise error. 
*
* Data is stored into the LIS3LV02DL data structure
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int lis3lv02dl_calib(LIS3LV02DL* acc)
{
    int err; 

    //TODO: THIS SHOULD AVERAGE A BIT MORE
    if(( err = lis3lv02dl_read(acc) ) < 0 )
        return err; 
    
    acc->xcal = acc->xacc;
    acc->ycal = acc->yacc;
    acc->zcal = acc->zacc;

    return 0;
}

/**
* @brief Initialized the LIS3LV02DL in 3-axis mode
*
* @param acc LIS3LV02DL accelerometer
* @return 0 on success. Otherwise error. 
*
* Initialized the LIS3LV02DL in 3-axis mode, 40hz
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int lis3lv02dl_init_3axis(LIS3LV02DL* acc)
{
    // TODO: SETTING BDU? OTHER THINGS...
    int err;
    int offx,offy,offz;
    
    util_pdbg(DBG_INFO,"LIS3LV02DL: Initializing accelerometer in 3-axis 6G Scale\n");
    
    if ( lis3lv02dl_id_check(acc) < 0){		
	util_pdbg(DBG_WARN,"LIS3LV02DL: Couldn't verify ID\n");
	return -ENODEV;
    }
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);
    
    /* Power-up device, Decimation 512 (40hz) and XYZ enabled */
    err = i2c_set( acc->i2c, acc->address, 
		  LIS3_REG_CTRLREG1 ,
		   'b' ,
                  LIS3_REG_CTRLREG1_PD_ON |
                  LIS3_REG_CTRLREG1_DEC_512 |
                  LIS3_REG_CTRLREG1_XEN_MASK |
                  LIS3_REG_CTRLREG1_YEN_MASK |		  
                  LIS3_REG_CTRLREG1_ZEN_MASK);
		  
    if( err < 0 ){
        util_pdbg(DBG_WARN,"LIS3LV02DL: Error when initializating for 3-axis\n");
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
        return err;
    }

    /* Scale:6g, BDU not continues, Little-endian,
     TODO:Interrupt disabled, 3-wire?, data 16 bit left */
    err = i2c_set( acc->i2c, acc->address, 
		  LIS3_REG_CTRLREG2,
		   'b' ,
                  LIS3_REG_CTRLREG2_FS_MASK | 
                  LIS3_REG_CTRLREG2_BDU_MASK |
                  LIS3_REG_CTRLREG2_DAS_MASK | 
                  LIS3_REG_CTRLREG2_BOOT_MASK | // TODO: Needed?
                  LIS3_REG_CTRLREG2_SIM_MASK);
    
    //TODO: shouldn't offX be int8_t? 
    offx = i2c_get(acc->i2c, acc->address, LIS3_REG_OFFSETX , 'b' );
    offy = i2c_get(acc->i2c, acc->address, LIS3_REG_OFFSETY , 'b' );
    offz = i2c_get(acc->i2c, acc->address, LIS3_REG_OFFSETZ , 'b' );
    
    util_pdbg(DBG_INFO, "OFFSETS: x=%d\ty=%d\tz%d\n",offx,offy,offz);
    
    if( err < 0 ){
        util_pdbg(DBG_WARN,"LIS3LV02DL: Error when initializating for 3-axis\n");
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
        return err;
    }
    
    UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
    
    return 0;
}
