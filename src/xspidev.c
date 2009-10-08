/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
            Based in code from the Linux Kernel documentation 

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

    NOTE: Done. Untested
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

int spi_full_transfer(xspidev* xspi, uint8_t* tx, uint8_t* rx, int len)
{
    int ret;

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	//.len = ARRAY_SIZE(tx),
	.len = len,
	.delay_usecs = xspi->delay,
	.speed_hz = xspi->speed,
	.bits_per_word = xspi->bits,
    };	
    
    if ((ret = ioctl(xspi->fd, SPI_IOC_MESSAGE(1), &tr)) == 1){
	util_pdbg(DBG_WARN,"SPI: Can't send spi message. Error %d", ret);
	return -EIO; 
    }
    
    #if DBG_LEVEL == 5
    for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
	if (!(ret % 6))
		puts(""); //TODO:CHANGE!
	util_pdbg(DBG_DEBG, "%.2X ", rx[ret]);
    }
    #endif
    
    return 0; 	
}

inline int spi_half_tranfer( xspidev* xspi, uint8_t* data, int len ) 
{
    int status; 
    if ( ( status = write( xspi->fd, data , len ) < 0 )){
            util_pdbg(DBG_WARN, "SPI: Error writing to device %s\n",xspi->device); 
            return -EIO;
    }
    return status; // written bytes
}

inline int spi_half_read( xspidev* xspi , uint8_t* data, int len ) 
{
    int status; 
    if ( ( status = read( xspi->fd, data , len ) < 0 )){
            util_pdbg(DBG_WARN, "SPI: Error reading from %s\n",xspi->device); 
            return -EIO;
    }
    return status; // read bytes
}


int spi_configure(xspidev* xspi, // STRUCT TO CONFIGURE
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

    return 0;
}

int spi_set_config(xspidev* xspi)
{
    int ret; 
    if ( ( xspi->fd = open(xspi->device, O_RDWR) ) < 0 ) 
        return -EBUSY; //can't open fd
    /*
     * spi mode
     */
    if( ( ret = ioctl(xspi->fd, SPI_IOC_WR_MODE, &xspi->mode) ) < 0 )
        return -EIO; //"can't set spi mode" 

    if( ( ret = ioctl(xspi->fd, SPI_IOC_RD_MODE, &xspi->mode) ) < 0 )
        return -EIO; //"can't get spi mode"
    /*
     * bits per word
     */
    if( (ret = ioctl(xspi->fd, SPI_IOC_WR_BITS_PER_WORD, &xspi->bits)) < 0 )
        return -EIO; //"can't set bits per word"

    if( (ret = ioctl(xspi->fd, SPI_IOC_RD_BITS_PER_WORD, &xspi->bits)) < 0 )
        return -EIO; //can't get bits per word
    /*
     * max speed hz
     */
    if( (ret = ioctl(xspi->fd, SPI_IOC_WR_MAX_SPEED_HZ, &xspi->speed)) < 0 )
        return -EIO;//can't set max speed hz

    if( (ret = ioctl(xspi->fd, SPI_IOC_RD_MAX_SPEED_HZ, &xspi->speed)) < 0 )
        return -EIO; // can't get max speed hz


    #if DBG_LEVEL == 5
    util_pdbg(DBG_DEBG, "SPI: mode: %d\n", xspi->mode);
    util_pdbg(DBG_DEBG, "SPI: bits per word: %d\n", xspi->bits);
    util_pdbg(DBG_DEBG, "SPI: max speed: %d Hz (%d KHz)\n", xspi->speed, xspi->speed/1000);
    #endif
    
    return 0;
}


