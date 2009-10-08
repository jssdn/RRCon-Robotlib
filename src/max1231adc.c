/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: max1231adc.c Driver for MAX1231/1230 ADCs

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

    NOTE: To be completed. Untested
*  ******************************************************************************* **/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
//Xenomai
#include <native/mutex.h>

#include "max1231adc.h"
#include "xspidev.h"
#include "util.h"

//TODO: Change timming!
// Look-up table for delay times for Clock 01. Have slightly more than 10% of safe margin and rounded up (in us)
int dtable[] = { 0 , 10, 19, 28 , 38 , 47 , 57 ,
                 66, 76 , 85 , 95 , 104 , 114 ,
                 123 , 133 , 142 , 152 , 161 ,
                 171 ,180 , 190 , 199 , 209 , 218 ,
                 227, 237 , 246, 256, 265 , 275 ,
                 284 , 293 , 303 }; // up to 32 samples , others extrapolate 


/* Config ADC inputs in different ways according to 'conf' */
//NOTE: The caller should assure that the device is not accessed externally through a mutex/other somewhere else. Mutual exclusion is just guaranteed over the same xspidev structure. 
int adc_config(xspidev* xspi, max1231_config* conf)
{
    int ret,i; 
    // sanity check? 
    uint8_t tx[8] = {0, }; // clock config + unipòlar config + 16 SCK clocks + clock config + bipolar config + 16 SCK clocks
    tx[0] = conf->clock | MAX1231_SETUP_UNIDIFF; 
    tx[4] = conf->clock | MAX1231_SETUP_BIPDIFF; 

    for( i = 0 ; i < 8 ; i++ )
    {
      if((conf->pairs[i] & MAX1231_CONF_BIPDIFF_MASK) != 0 )
	  tx[5] = conf->pairs[i] >> 1 ; //i/2
      if((conf->pairs[i] & MAX1231_CONF_UNIDIFF_MASK) != 0 ) // By Spec, Unipolar take predecence over bipolar, there is no need of sanity check here
	  tx[1] = conf->pairs[i] >> 1 ; //i/2
     }

     uint8_t rx[ARRAY_SIZE(tx)] = {0, };  

    struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)tx,
            .rx_buf = (unsigned long)rx,
            .len = ARRAY_SIZE(tx),
            .delay_usecs = xspi->delay,
            .speed_hz = xspi->speed,
            .bits_per_word = xspi->bits,
    };

    ret = ioctl(xspi->fd, SPI_IOC_MESSAGE(1), &tr);
    
    if (ret == 1){
            util_pdbg(DBG_WARN, "MAX1231: Can't send spi message");
            return -1;
    }

    return 0;
}

/* TODO: All the following calls look the same. Optimize! */

/* TODO: Take it out from here */
#define CMD_ALL_SINGLE_TX 0x64
#define CMD_ALL_SINGLE_SLEEP 1000

#define CMD_ALL_DIFF_TX 0x64
#define CMD_ALL_DIFF_SLEEP 1000

#define MAX1231_RESET_ALL_SLEEP 100

/* Low level write for 8bits to support genral commands*/
int adc_ll_write8(xspidev* xspi, uint8_t tx, int sleep) 
{
    int err;

    UTIL_MUTEX_ACQUIRE("MAX1231",&(xspi->mutex),TM_INFINITE);
    
    err = write( xspi->fd, &tx , 1 );

    UTIL_MUTEX_RELEASE("MAX1231",&(xspi->mutex));

    if ( err < 0 ){
	util_pdbg(DBG_WARN, "Error writing to device %s\n",xspi->device); 
	return -EIO;
    }

    //TODO: Calculate properly
    __usleep(sleep);

    return 0;    
}

/* Reads an array of 'len' bytes to 'dest_array' */
int adc_read(xspidev* xspi,uint8_t convbyte,uint8_t* dest_array, int len)
{
    int err,err2;
    int i ; 

    UTIL_MUTEX_ACQUIRE("MAX1231",&(xspi->mutex),TM_INFINITE);

    err = write( xspi->fd, &convbyte , 1 );
    
    //Check times in a look-up table according to len 
    if( convbyte & MAX1231_CONV_TEMP )
	__usleep( dtable[(len>>1)] + MAX1231_DELAY_TEMP );
    else 
	__usleep( dtable[(len>>1)] );

// 	usleep(200); // should be less 
    
    err2 = read( xspi->fd, dest_array , len );

    UTIL_MUTEX_RELEASE("MAX1231",&(xspi->mutex));
    
    if ( err2 < 0 || err < 0 ){
	util_pdbg(DBG_WARN,"MAX1231: Error reading/writing from/to device %s. Read:%d Write:%d\n",xspi->device,err2,err); 
	return err;
    }

    #if DBG_LEVEL == 5
    for( i = 0 ; i < len ; i ++){
	util_pdbg(DBG_DEBG, "Read[%d]: 0x%x\n", i, dest_array[i]);
    }
    #endif
    
    return 0; 
}

/* Read one two bytes measure from the ADC */
int adc_read_one_once(xspidev* xspi, uint8_t n, int* ret)
{
    int err; 
    uint8_t convbyte;
    uint8_t dest[2];

    if( n > 16) 
	    return -1; // out of bounds

    if( n == 16 ) //temperature 
	convbyte = 0x80 | ( n << 3 ) | MAX1231_CONV_SINGLE_READ | MAX1231_CONV_TEMP ; 
    else 
	convbyte = 0x80 | ( n << 3 ) | MAX1231_CONV_SINGLE_READ ;
    
    UTIL_MUTEX_ACQUIRE("MAX1231",&(xspi->mutex),TM_INFINITE);
    
    err = adc_read(xspi, convbyte, dest, 2 );
    
    UTIL_MUTEX_RELEASE("MAX1231",&(xspi->mutex));
    
    if ( err < 0 ){
	util_pdbg(DBG_WARN, "MAX1231:Error reading from device %s. Error: %d\n",xspi->device,err); 
	return -EIO;
    }


    *ret = ( (int)(dest[0] << 8 ) | (int)dest[1] ); 
    
    return 0;
}

/* Reads in Scan mode from byte 0 to N */
int adc_read_scan_0_N(xspidev* xspi, uint8_t* dest, uint8_t n)
{
    int len = ((n & 0x0f) << 1) + 2; // n*2 + 2
    uint8_t convbyte;

	if( n > 15) 
	    return -1; // out of bounds

    convbyte = 0x80 | ( (n & 0x0f)  << 3 ) | MAX1231_CONV_SCAN_00_N ;

    return adc_read(xspi, convbyte, dest, len );
}

/* Simplify reading of temperature and returns value in degrees */
// Needs to divide by 8 afterwards
int adc_get_temperature(xspidev* xspi, int* ret)
{
    int err;
    
    if((err= adc_read_one_once( xspi, 16, ret))<0){
	util_pdbg(DBG_INFO, "MAX1231: Couldn't get temperature . Error : %d \n", err);
	return err;
    }
    
    *ret = *ret;
    return 0;
}
