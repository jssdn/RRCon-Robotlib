/**
    @file max1231adc.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Driver for MAX1231/1230 ADCs
    
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


/**
* @brief Initialization for the MAX1231 device
*
* @param adc MAX1231 device to use
* @param spi Already initialized SPIDEV device to use
* @return 0 on success. Otherwise error. 
*
* Initizializes the structure and creates corresponding mutex 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int max1231_init(MAX1231* adc, XSPIDEV* spi)
{
    int err; 

    util_pdbg(DBG_INFO, "Initializing MAX1231 ADC...\n");
    
    if( spi == NULL || adc == NULL ){	
	util_pdbg(DBG_WARN, "MAX1231: Cannot use non-initialized devices...\n");
	return -EFAULT; 
    }

    adc->xspi = spi; 
    //TODO: configure the pairs for later on calling max1231_config
    //--

    UTIL_MUTEX_CREATE("MAX1231",&(adc->mutex), NULL);

    return 0; 
}

/**
* @brief Cleans the MAX1231 device
*
* @param adc MAX1231 device to clena
* @return 0 on success. Otherwise error. 
*
* Cleans structure and Mutex
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int max1231_clean(MAX1231* adc)
{
    int err; 

    util_pdbg(DBG_INFO, "Cleaning the MAX1231 ADC...\n");
    
    adc->xspi = NULL;
    
    UTIL_MUTEX_DELETE("MAX1231", &(adc->mutex));
    
    return err; 

}

/**
* @brief Configs the MAX1231 device
*
* @param adc MAX1231 device to clena
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int max1231_config(MAX1231* adc)
{
    int err,i; 
    // sanity check? 
    uint8_t tx[8] = {0, }; // clock config + unipolar config + 16 SCK clocks + clock config + bipolar config + 16 SCK clocks
    uint8_t rx[ARRAY_SIZE(tx)] = {0, };  
    tx[0] = adc->clock | MAX1231_SETUP_UNIDIFF; 
    tx[4] = adc->clock | MAX1231_SETUP_BIPDIFF; 

    for( i = 0 ; i < 8 ; i++ )
    {
      if((adc->pairs[i] & MAX1231_CONF_BIPDIFF_MASK) != 0 )
	  tx[5] = adc->pairs[i] >> 1 ; //i/2
      if((adc->pairs[i] & MAX1231_CONF_UNIDIFF_MASK) != 0 ) // By Spec, Unipolar take predecence over bipolar, there is no need of sanity check here
	  tx[1] = adc->pairs[i] >> 1 ; //i/2
     }
     
    if( (err = spi_full_transfer(adc->xspi, tx, rx, ARRAY_SIZE(tx))) < 0 )
    {
	util_pdbg(DBG_WARN, "MAX1231: Can't config MAX1231");
	return err; 
    }
	
    return 0; 
}

/**
* @brief Write for 8bits commands
*
* @param adc MAX1231 device to clena
* @param tx TX buffer for the max1231
* @param sleep 
* @return 0 on success. Otherwise error. 
*
* Low level write for 8bits to support general commands
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int adc_ll_write8(MAX1231* adc, uint8_t tx, int sleep) 
{
    int err;

    UTIL_MUTEX_ACQUIRE("MAX1231",&(adc->mutex),TM_INFINITE);
    
    err = spi_half_transfer(adc->xspi, &tx , 1 );
			     
    UTIL_MUTEX_RELEASE("MAX1231",&(adc->mutex));

    if ( err < 0 ){
	util_pdbg(DBG_WARN, "MAX1231: Error writing to device %s\n",(adc->xspi)->device); 
	return -EIO;
    }

    //TODO: Calculate properly
    __usleep(sleep);

    return 0;    
}

/**
* @brief Reads from MAX1231
*
* @param adc MAX1231 device to clena
* @param convbyte Conversion byte 
* @param dest_array Destination array for readings
* @param len Length of dest_array
* @return 0 on success. Otherwise error. 
*
* Reads an array of 'len' bytes to 'dest_array'
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int adc_read(MAX1231* adc,uint8_t convbyte,uint8_t* dest_array, int len)
{
    int err,err2;
    #ifdef DBG_LL_SPI
    int i ; 
    #endif

    UTIL_MUTEX_ACQUIRE("MAX1231",&(adc->mutex),TM_INFINITE);

    err = spi_half_transfer(adc->xspi, &convbyte , 1 );
    
    //Check times in a look-up table according to len 
    if( convbyte & MAX1231_CONV_TEMP )
	__usleep( dtable[(len>>1)] + MAX1231_DELAY_TEMP );
    else 
	__usleep( dtable[(len>>1)] );

    // -- !!!!TODO: REMOVE!
    __usleep(200); // should be less 
    // -- 
    
    err2 = spi_half_read(adc->xspi , dest_array, len);

    UTIL_MUTEX_RELEASE("MAX1231",&(adc->mutex));
    
    if ( err2 < 0 || err < 0 ){
	util_pdbg(DBG_WARN,"MAX1231: Error reading/writing from/to device %s. Read:%d Write:%d\n",(adc->xspi)->device,err2,err); 
	return -EIO;
    }

    #ifdef DBG_LL_SPI
     for( i = 0 ; i < len ; i ++){
 	util_pdbg(DBG_DEBG, "Read[%d]: 0x%x\n", i, dest_array[i]);
     }
    #endif
    
    return 0; 
}

/**
* @brief Reads one measure MAX1231
*
* @param adc MAX1231 device to clena
* @param n Channel number
* @param ret Read value
* @return 0 on success. Otherwise error. 
*
* Read one 16 bit ( actual 12 ) measure from the ADC
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int adc_read_one_once(MAX1231* adc, uint8_t n, int* ret)
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
    
                
    if ((err = adc_read(adc, convbyte, dest, 2 )) < 0 ){
	util_pdbg(DBG_WARN, "MAX1231:Error reading from device %s. Error: %d\n",adc->xspi->device, err); 
	return -EIO;
    }
    
    *ret = ( (int)(dest[0] << 8 ) | (int)dest[1] ); 
    
    return 0;
}

/**
* @brief Reads n measures from 0 to n
*
* @param adc MAX1231 device to clena
* @param n Channel number
* @param dest Destination array
* @return 0 on success. Otherwise error. 
*
* Reads in Scan mode from byte 0 to N
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int adc_read_scan_0_N(MAX1231* adc, uint8_t* dest, uint8_t n)
{
    int len = ((n & 0x0f) << 1) + 2; // n*2 + 2
    uint8_t convbyte;

    if( n > 15) 
	return -1; // out of bounds

    convbyte = 0x80 | ( (n & 0x0f)  << 3 ) | MAX1231_CONV_SCAN_00_N ;

//     return adc_read(adc, convbyte, adc->dest, len );
    return adc_read(adc, convbyte, dest, len );
}


/**
* @brief Reads the temperature from the MAX1231
*
* @param adc MAX1231 device to clena
* @param n Channel number
* @param dest Destination array
* @return 0 on success. Otherwise error. 
*
* Reads the temperature from the internal sensor in the MAX1231
* Simplify reading of temperature and returns value in degrees
*
* @note Needs to downscale(divide) by 8 afterwards
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int adc_get_temperature(MAX1231* adc, int* ret)
{
    int err;
    
    if((err= adc_read_one_once( adc, 16, ret))<0){
	util_pdbg(DBG_INFO, "MAX1231: Couldn't get temperature . Error : %d \n", err);
	return err;
    }
    
    return 0;
}

