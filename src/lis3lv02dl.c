/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: lis3lv02dl.c Driver for I2C Accelerometer lis3lv02dl.c

    License: Licensed under GPL2.0 

    Notes:  See application note AN2381 ST 
*  ******************************************************************************* */

#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>

#include "lis3lv02dl.h"
#include "i2ctools.h"

inline int lis3lv02dl_id_check(uint8_t address,int i2cbus)
{
    return i2cget(address, LIS3_REG_WHOAMI, i2cbus, 'b' ) == LIS3_ID? 0 : -1 ;
}


int lis3lv02dl_poweroff(uint8_t address, int i2cbus)
{
    return i2cset(address, LIS3_REG_CTRLREG1, LIS3_REG_CTRLREG1_PD_OFF , i2cbus, 'b' );
}

int lis3lv02dl_read(uint8_t address, int i2cbus, lis3s* lis3data)
{
    int res,i;
    uint16_t tmp[6];

    if ( ( res = i2cget(address, LIS3_REG_STATUSREG, i2cbus, 'b' ) ) < 0 ) {
        printf("Cannot read 0x%x from LIS3LC02DL\n", LIS3_REG_STATUSREG ) ; 
        return res; 
    }

    if( !(res & LIS3_REG_STATUSREG_ZYXDA_MASK) )
        return -EAGAIN; // No new data available

    if( (res & LIS3_REG_STATUSREG_ZYXOR_MASK) )
        lis3data->data_overrun = 1;
    else 
        lis3data->data_overrun = 0;

   //TODO:more efficient by reading all the bytes continously 
   for( i = 0 ; i < 6 ; i++ )
   {
//         if(( tmp[i] = i2cget(address, LIS3_REG_OUTX_L + i , i2cbus, 'b' )) < 0) 
//             return tmp[i]; //i2c error
        tmp[i] = i2cget(address, LIS3_REG_OUTX_L + i , i2cbus, 'b' );

   }

    // remove lower 4 bits ( noise ) 
    lis3data->xacc = (tmp[0] & 0xf0)| (tmp[1] << 8);     
    lis3data->yacc = (tmp[2] & 0xf0)| (tmp[3] << 8);
    lis3data->zacc = (tmp[4] & 0xf0)| (tmp[5] << 8);

   return 0;
}

int lis3lv02dl_calib(uint8_t address, int i2cbus, lis3s* lis3data)
{
    int res; 

    if(( res = lis3lv02dl_read(address, i2cbus, lis3data) ) < 0 )
        return res; 

    lis3data->xcal = lis3data->xacc;
    lis3data->ycal = lis3data->yacc;
    lis3data->zcal = lis3data->zacc;

    return 0;
}

int lis3lv02dl_init_3axis(uint8_t address, int i2cbus)
{
    // TODO: SETTING BDU? OTHER THINGS...
    int res;
    int offx,offy,offz;
    
    if ( lis3lv02dl_id_check(address,i2cbus) < 0){		
		return -ENODEV;
    }

    /* Power-up device, Decimation 512 (40hz) and XYZ enabled */
    res = i2cset(address, LIS3_REG_CTRLREG1 ,
                  LIS3_REG_CTRLREG1_PD_ON |
                  LIS3_REG_CTRLREG1_DEC_512 |
                  LIS3_REG_CTRLREG1_XEN_MASK |
                  LIS3_REG_CTRLREG1_YEN_MASK |
                  LIS3_REG_CTRLREG1_ZEN_MASK,
                  i2cbus,
                  'b' );
    if( res < 0 ){
        printf("LIS3LV02DL: Error in initialization 1\n");
        return res;
    }

    /* Scale:6g, BDU not continues, Little-endian,
     TODO:Interrupt disabled, 3-wire?, data 16 bit left */
    res = i2cset(address, LIS3_REG_CTRLREG2,
                  LIS3_REG_CTRLREG2_FS_MASK | 
                  LIS3_REG_CTRLREG2_BDU_MASK |
                  LIS3_REG_CTRLREG2_DAS_MASK | 
                  LIS3_REG_CTRLREG2_BOOT_MASK | // TODO: Needed?
                  LIS3_REG_CTRLREG2_SIM_MASK,
                  i2cbus,
                  'b' );
    
    offx = i2cget(address, LIS3_REG_OFFSETX , i2cbus, 'b' );
    offy = i2cget(address, LIS3_REG_OFFSETY , i2cbus, 'b' );
    offz = i2cget(address, LIS3_REG_OFFSETZ , i2cbus, 'b' );
    printf("OFFSETS: x=%d\ty=%d\tz%d\n",offx,offy,offz);
    
    if( res < 0 ){
        printf("LIS3LV02DL: Error in initialization 1\n");
        return res;
    }

    return 0;
}
