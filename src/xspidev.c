/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author: Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
            Based in code from the Linux Kernel Documentation 

    Code: xspidev.c SPIDEV primitive functions (full/half duplex)

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "xspidev.h"
#include "util.h" 

/* Not thread-safe */
static int spi_set_config(XSPIDEV* xspi)
{
    int err; 
   
    if ( ( xspi->fd = open(xspi->device, O_RDWR) ) < 0 ) 
        return -EBUSY; //can't open fd
    /*
     * spi mode
     */
    if( ( err = ioctl(xspi->fd, SPI_IOC_WR_MODE, &xspi->mode) ) < 0 )
        return -EIO; //"can't set spi mode" 

    if( ( err = ioctl(xspi->fd, SPI_IOC_RD_MODE, &xspi->mode) ) < 0 )
        return -EIO; //"can't get spi mode"
    /*
     * bits per word
     */
    if( (err = ioctl(xspi->fd, SPI_IOC_WR_BITS_PER_WORD, &xspi->bits)) < 0 )
        return -EIO; //"can't set bits per word"

    if( (err = ioctl(xspi->fd, SPI_IOC_RD_BITS_PER_WORD, &xspi->bits)) < 0 )
        return -EIO; //can't get bits per word
    /*
     * max speed hz
     */
    if( (err = ioctl(xspi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &xspi->speed)) < 0 )
        return -EIO;//can't set max speed hz

    if( (err = ioctl(xspi->fd, SPI_IOC_RD_MAX_SPEED_HZ, &xspi->speed)) < 0 )
        return -EIO; // can't get max speed hz

    util_pdbg(DBG_DEBG, "SPI: mode: %d\n", xspi->mode);
    util_pdbg(DBG_DEBG, "SPI: bits per word: %d\n", xspi->bits);
    util_pdbg(DBG_DEBG, "SPI: max speed: %d Hz (%d KHz)\n", xspi->speed, xspi->speed/1000);
    
    return 0;
}

int spi_init(	XSPIDEV* xspi, // STRUCT TO CONFIGURE
		const char* devname, // DEVICE NAME 
		uint32_t speed, // SPEED IN HZ
		uint16_t delay, // DELAY IN ¿¿??
		uint8_t bits, // BITS AT A TIME
		uint8_t loop, // LOOPBACK MODE
		uint8_t cpha, // CPHA
		uint8_t cpol, // CPOL 
		uint8_t lsb_first, // LSB FIRST
		uint8_t cs_high, // CS HIGH
		uint8_t spi_3wire) // 3 WIRE SPI MODE
{
    int err; 
    
    if ( devname == NULL || strlen(devname) >= DEVBUFFERSIZE ) //check for possible buffer overflow 
        return -ENODEV; 

    strncpy(xspi->device, devname, DEVBUFFERSIZE) ;
    
    if( (xspi->fd = open(xspi->device, O_RDWR)) == 0) 
        return -EBUSY; //cannot open device

    xspi->speed = speed ? speed : SPI_DEFAULT_SPEED ;
    xspi->delay = delay ? delay : SPI_DEFAULT_DELAY ;
    xspi->bits = bits ? bits : SPI_DEFAULT_BITS ;

    if( loop > 0 ) 
        xspi->mode |= SPI_LOOP;
    if( cpha > 0 ) 
        xspi->mode |= SPI_CPHA ;
    if( cpol > 0 ) 
        xspi->mode |= SPI_CPOL ;
    if( lsb_first > 0 ) 
        xspi->mode |= SPI_LSB_FIRST ;
    if( cs_high > 0 ) 
        xspi->mode |= SPI_CS_HIGH ;
    if( spi_3wire > 0 ) 
        xspi->mode |= SPI_3WIRE ;

    UTIL_MUTEX_CREATE("SPI",&(xspi->mutex), NULL);

    return spi_set_config(xspi);     
}

int spi_clean(XSPIDEV* xspi)
{
    int err; 
    
    UTIL_MUTEX_DELETE("SPI", &(xspi->mutex));
    
    return 0; 
}

int spi_full_transfer(XSPIDEV* xspi, uint8_t* tx, uint8_t* rx, int len)
{
    int err;

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	//.len = ARRAY_SIZE(tx),
	.len = len,
	.delay_usecs = xspi->delay,
	.speed_hz = xspi->speed,
	.bits_per_word = xspi->bits,
    };	
    
    UTIL_MUTEX_ACQUIRE("SPI",&(xspi->mutex),TM_INFINITE);
    
    err = ioctl(xspi->fd, SPI_IOC_MESSAGE(1), &tr);
       
    UTIL_MUTEX_RELEASE("SPI",&(xspi->mutex));
    
    if ( err == 1){
	util_pdbg(DBG_WARN,"SPI: Can't send spi message. Error %d", err);
	return -EIO; 
    }
    
    #ifdef DBG_LL_SPI
    for (err = 0; err < ARRAY_SIZE(tx); err++) {
	if (!(err % 6))
		puts(""); //TODO:CHANGE!
	util_pdbg(DBG_DEBG, "%.2X ", rx[err]);
    }
    #endif
    
    return 0; 	
}

int spi_half_transfer( XSPIDEV* xspi, uint8_t* data, int len ) 
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("SPI",&(xspi->mutex),TM_INFINITE);
    
    err = write( xspi->fd, data , len );
    
    UTIL_MUTEX_RELEASE("SPI",&(xspi->mutex));
    
    if( err < 0 ){
	util_pdbg(DBG_WARN, "SPI: Error writing to device %s\n",xspi->device); 
	return -EIO;
    }
    
    return err; // written bytes
}

int spi_half_read( XSPIDEV* xspi , uint8_t* data, int len ) 
{
    int err; 
    
    UTIL_MUTEX_ACQUIRE("SPI",&(xspi->mutex),TM_INFINITE);
    
    err = read( xspi->fd, data , len ); 
    
    UTIL_MUTEX_RELEASE("SPI",&(xspi->mutex));
    
    if ( err < 0 ){
	util_pdbg(DBG_WARN, "SPI: Error reading from %s\n",xspi->device); 
	return -EIO;
    }    
    
    return err; // read bytes
}
