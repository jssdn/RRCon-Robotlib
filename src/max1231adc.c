/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: max1231adc.c Driver for MAX1231/1230 ADCs

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "max1231adc.h"
#include "xspidev.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// Look-up table for delay times for Clock 01. Have slightly more than 10% of safe margin and rounded up (in us)
int dtable[] = { 0 , 10, 19, 28 , 38 , 47 , 57 ,
                 66, 76 , 85 , 95 , 104 , 114 ,
                 123 , 133 , 142 , 152 , 161 ,
                 171 ,180 , 190 , 199 , 209 , 218 ,
                 227, 237 , 246, 256, 265 , 275 ,
                 284 , 293 , 303 }; // up to 32 samples , others extrapolate 


/* Config ADC inputs in different ways according to 'conf' */
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
            printf("can't send spi message");
            return -1;
    }

    return 0;
}

/* Config all inputs in unipolar single-ended mode */
int adc_config_all_uni_single(xspidev* xspi)
{
     int status;

     uint8_t tx = 0x64; // clock 10 (internal, no #cnst), ref 01 (external,no wake-up delay) , diff 00 (unipolar/bipolar unchanged)

    if ( ( status = write( xspi->fd, &tx , 1 ) < 0 )){
            printf("Error writing to device %s\n",xspi->device); 
            return -1;
    }

    usleep(1000);

    return 0;
}

/* Config a pair('first', second)  in differential mode */
int adc_config_diff(xspidev* xspi, uint8_t first) 
{
    int status;

    uint8_t tx = 0x64; // clock 10 (internal, no #cnst), ref 01 (external,no wake-up delay) , diff 00 (unipolar/bipolar unchanged)

    if ( ( status = write( xspi->fd, &tx , 1 ) < 0 )){
            printf("Error writing to device %s\n",xspi->device); 
            return -1;
    }

    usleep(1000);

    return 0;
}

/* Reset device and FIFO */
void adc_reset(xspidev* xspi)
{
    int status; 
    uint8_t rst = MAX1231_RESET_ALL; 

    if ( ( status = write( xspi->fd, &rst , 1 ) < 0 )){
            printf("Error writing to device %s\n",xspi->device); 
            return;
    }
    usleep(100); // TODO: needed?
}

/* Reset FIFO */
void adc_reset_fifo(xspidev* xspi)
{
    int status; 
    uint8_t rst = MAX1231_RESET_FIFO;
    if ( ( status = write( xspi->fd, &rst , 1 ) < 0 )){
            printf("Error writing to device %s\n",xspi->device); 
            return;
    }
//     usleep(100);
}

/* Reads an array of 'len' bytes to 'dest_array' */
int adc_read(xspidev* xspi,uint8_t convbyte,uint8_t* dest_array, int len)
{
	int status;
	int i ; 
	
	if ( ( status = write( xspi->fd, &convbyte , 1 ) < 0 )){
                printf("Error writing to device %s\n",xspi->device); 
                return -2;
        }
        //Check times in a look-up table according to len 
        if( convbyte & MAX1231_CONV_TEMP )
            usleep( dtable[(len>>1)] + MAX1231_DELAY_TEMP );
        else 
            usleep( dtable[(len>>1)] );

// 	usleep(200); // should be less 
	
	if ( ( status = read( xspi->fd, dest_array , len ) < 0 )) {
            printf("Error reading from device %s\n",xspi->device);
            return -3;
        }

        #ifdef DEBUGMODE
        for( i = 0 ; i < len ; i ++){
            printf( "Read[%d]: 0x%x\n", i, dest_array[i]);
        }
        #endif
        return 0; 
}

/* Read one two bytes measure from the ADC */
int adc_read_one_once(xspidev* xspi,uint8_t n)
{
        int ret; 
        uint8_t convbyte;
        uint8_t dest[2];

        if( n > 16) 
             return -1; // out of bounds

  	if( n == 16 ) //temperature 
  	     convbyte = 0x80 | ( n << 3 ) | MAX1231_CONV_SINGLE_READ | MAX1231_CONV_TEMP ; 
  	else 
  	     convbyte = 0x80 | ( n << 3 ) | MAX1231_CONV_SINGLE_READ ;

 	if ( (ret = adc_read(xspi, convbyte, dest, 2 )) < 0 ) 
 	   return ret; 

        return ( (int)(dest[0] << 8 ) | (int)dest[1] ); 
}

/* Reads in Scan mode from byte 0 to N */
int adc_read_scan_0_N(xspidev* xspi,uint8_t* dest, uint8_t n)
{
        int len = ((n & 0x0f) << 1) + 2; // n*2 + 2
        uint8_t convbyte;

         if( n > 15) 
             return -1; // out of bounds

        convbyte = 0x80 | ( (n & 0x0f)  << 3 ) | MAX1231_CONV_SCAN_00_N ;

        return adc_read(xspi, convbyte, dest, len );
}

/* Simplify reading of temperature and returns value in degrees */
inline double adc_get_temperature(xspidev* xspi)
{
    return ((double)adc_read_one_once( xspi, 16))/8;
}
