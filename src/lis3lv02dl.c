/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: lis3lv02dl.c Driver for I2C Accelerometer lis3lv02dl.c
    Notes:  See application note AN2381 ST 

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

    NOTE:Done. Untested
*  ******************************************************************************* **/


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

int lis3lv02dl_clean(LIS3LV02DL* acc)
{
    int err; 

    util_pdbg(DBG_INFO, "Cleaning the LIS3LV02DL accelerometer...\n");
    
    acc->i2c = NULL;
    acc->address = 0x00; 
    
    err = lis3lv02dl_poweroff(acc); 
    
    UTIL_MUTEX_DELETE("LIS3LV02DL", &(acc->mutex));
    
    return err; 
}

int lis3lv02dl_id_check(LIS3LV02DL* acc)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);
        
    err = i2c_get(acc->i2c, acc->address, LIS3_REG_WHOAMI, 'b' );
    
    UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));

    return err == LIS3_ID? 0 : -1; 
}


int lis3lv02dl_poweroff(LIS3LV02DL* acc)
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);

    err = i2c_set(acc->i2c, acc->address, LIS3_REG_CTRLREG1, LIS3_REG_CTRLREG1_PD_OFF , 'b' );
    
    UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
    
    return err; 
}

int lis3lv02dl_read(LIS3LV02DL* acc)
{
    int err,i; 
    uint16_t tmp[6];
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);

    if ( ( err = i2c_get(acc->i2c, acc->address, LIS3_REG_STATUSREG, 'b' ) ) < 0 ) {
        util_pdbg(DBG_WARN, "Cannot read 0x%x from LIS3LC02DL\n", LIS3_REG_STATUSREG ) ; 
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
        return err; 
    }

    if( !(err & LIS3_REG_STATUSREG_ZYXDA_MASK) ){
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
        return -EAGAIN; // No new data available
    }
    
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

int lis3lv02dl_calib(LIS3LV02DL* acc)
{
    int err; 

    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);
    
    if(( err = lis3lv02dl_read(acc) ) < 0 ){
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
        return err; 
    }
    
    acc->xcal = acc->xacc;
    acc->ycal = acc->yacc;
    acc->zcal = acc->zacc;

    UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));

    return 0;
}

int lis3lv02dl_init_3axis(LIS3LV02DL* acc)
{
    // TODO: SETTING BDU? OTHER THINGS...
    int err;
    int offx,offy,offz;
    
    util_pdbg(DBG_INFO,"LIS3LV02DL: Initializing accelerometer in 3-axis 6G Scale\n");
    
    UTIL_MUTEX_ACQUIRE("LIS3LV02DL",&(acc->mutex),TM_INFINITE);
    
    if ( lis3lv02dl_id_check(acc) < 0){		
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
	return -ENODEV;
    }

    /* Power-up device, Decimation 512 (40hz) and XYZ enabled */
    err = i2c_set( acc->i2c, acc->address, 
		  LIS3_REG_CTRLREG1 ,
                  LIS3_REG_CTRLREG1_PD_ON |
                  LIS3_REG_CTRLREG1_DEC_512 |
                  LIS3_REG_CTRLREG1_XEN_MASK |
                  LIS3_REG_CTRLREG1_YEN_MASK |
                  LIS3_REG_CTRLREG1_ZEN_MASK,                  
                  'b' );
		  
    if( err < 0 ){
        util_pdbg(DBG_WARN,"LIS3LV02DL: Error when initializating for 3-axis\n");
	UTIL_MUTEX_RELEASE("LIS3LV02DL",&(acc->mutex));
        return err;
    }

    /* Scale:6g, BDU not continues, Little-endian,
     TODO:Interrupt disabled, 3-wire?, data 16 bit left */
    err = i2c_set( acc->i2c, acc->address, 
		  LIS3_REG_CTRLREG2,
                  LIS3_REG_CTRLREG2_FS_MASK | 
                  LIS3_REG_CTRLREG2_BDU_MASK |
                  LIS3_REG_CTRLREG2_DAS_MASK | 
                  LIS3_REG_CTRLREG2_BOOT_MASK | // TODO: Needed?
                  LIS3_REG_CTRLREG2_SIM_MASK,
                  'b' );
    
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