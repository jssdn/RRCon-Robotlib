/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
             Based in code from i2ctools-3.0.1 by Jean Delvare <khali@linux-fr.org>
                                                  Frodo Looijaard <frodol@dds.nl>
                                                  Mark D. Studebaker <mdsxyz123@yahoo.com>

    Code: i2ctools.c I2C Send/Receive functions

    License: Licensed under GPL2.0 

    NOTE: This code assumes capabilities of byte/word reading and are not checked.
	  Tested with Xilinx IIC Core and Bitbanged i2c gpio core.
    
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
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
//Xenomai
#include <native/mutex.h>
//--

#include "i2cbusses.h"
#include "i2ctools.h"
#include "util.h" 
#include "dev_mmaps_parms.h"

int i2c_init(I2CDEV* i2c, uint8_t bus)
{
    int err; 
    
    util_pdbg(DBG_DEBG, "I2C: Initializing I2C Device...\n");

    if (bus > 0x3f) {
	fprintf(stderr, "I2C: I2CBUS argument invalid!\n");
	return -ENODEV;
    }
    
    i2c->i2cbus = bus; 
    
    if( (i2c->file = open_i2c_dev(i2c->i2cbus, i2c->filename, 0)) < 0 ){
	util_pdbg(DBG_WARN, "I2C: File descriptor could not be allocated. Error %d\n", i2c->file);
	return -ENODEV; 
    }

    util_pdbg(DBG_INFO, "I2C: Succesfully open file in %s on bus %d\n",i2c->filename,i2c->i2cbus);

    UTIL_MUTEX_CREATE("I2C",&(i2c->mutex),NULL);
   
    return 0; 
}

int i2c_clean(I2CDEV* i2c)
{
    int err;

    util_pdbg(DBG_DEBG, "I2C: Cleaning I2C Device...\n");

    UTIL_MUTEX_DELETE("I2C",&(i2c->mutex));
    
    if( ( err = close(i2c->file)) < 0 )
	return err; 
    
    return 0;
}

/* SMBus I2C functions for read/write */

int i2c_get( I2CDEV* i2c, uint8_t address, uint8_t daddress, char csize)
{
    int res,err;	    
   
    if ( address < 3 || address > 0x77) {
	util_pdbg(DBG_WARN , "I2C: Chip address invalid!\n");
	return -EADDRNOTAVAIL;
    }

    UTIL_MUTEX_ACQUIRE("I2C",&(i2c->mutex),TM_INFINITE);
 
    set_slave_addr(i2c->file, address, 0);
   
    switch (csize)
    {
	case 'b': //I2C_SMBUS_BYTE
	    res = i2c_smbus_read_byte_data(i2c->file, daddress);
	    break;
	case 'w': //I2C_SMBUS_WORD_DATA; 
	    res = i2c_smbus_read_word_data(i2c->file, daddress);
	    break;
	case 'c': // I2C_SMBUS_BYTE_DATA 
	    res = i2c_smbus_read_byte_data(i2c->file, daddress);
	    break;
	default: 
	    res = -1; // INVALID DATA SIZE
    }
    
    UTIL_MUTEX_RELEASE("I2C",&(i2c->mutex));
    
    if (res < 0) {
	    fprintf(stderr, "Error: Read failed\n");
	    return -EIO;
    }
    
    #ifdef DBG_LL_I2C
    util_pdbg(DBG_DEBG, "0x%0*x\n", csize == 'w' ? 4 : 2, res);    
    #endif
    
    return res;
}

int i2c_set(I2CDEV* i2c, uint8_t address, uint8_t daddress, char csize, unsigned value)
{
    int res,err;
    
    if (address > 0x7f) {
	util_pdbg(DBG_WARN , "I2C: Chip address invalid!\n");
	return -EADDRNOTAVAIL;
    }

    UTIL_MUTEX_ACQUIRE("I2C",&(i2c->mutex),TM_INFINITE);
    
    set_slave_addr(i2c->file, address, 0);

    if (csize == 'w')
	res = i2c_smbus_write_word_data(i2c->file, daddress, value & 0xffff);
    else 
	res = i2c_smbus_write_byte_data(i2c->file, daddress, value & 0xff);
    
    UTIL_MUTEX_RELEASE("I2C",&(i2c->mutex));    
    
    if (res < 0) {
	util_pdbg(DBG_WARN,  "Error: Write failed\n");
	return -EIO;		
    }
    
    #ifdef DBG_LL_I2C
    util_pdbg(DBG_DEBG, "Written 0x%x on daddress:0x%x\n",value, daddress);
    #endif
    
    return 0; 
}

/* Special I2C functions for read/write with 3 parameters(see HMC6352) */
int i2c_get_3com( I2CDEV* i2c, uint8_t address, uint8_t arg1, uint8_t arg2)
{
    int err,res;	        

    UTIL_MUTEX_ACQUIRE("I2C",&(i2c->mutex),TM_INFINITE);
 
    // Write 3 arguments on the i2c bus
    i2c_set(i2c,address,arg1,'b', arg2);    
    set_slave_addr(i2c->file, address, 0);      
    res = i2c_smbus_read_byte(i2c->file);

    UTIL_MUTEX_RELEASE("I2C",&(i2c->mutex));
    
    return 0;
}

int i2c_set_1com( I2CDEV* i2c, uint8_t address, uint8_t arg1)
{
    int res,err;

    if (address > 0x7f) {
	util_pdbg(DBG_WARN , "I2C: Chip address invalid!\n");
	return -EADDRNOTAVAIL;
    }

    UTIL_MUTEX_ACQUIRE("I2C",&(i2c->mutex),TM_INFINITE);

    set_slave_addr(i2c->file, address, 0);
    res = i2c_smbus_write_byte(i2c->file,arg1);
    
    UTIL_MUTEX_RELEASE("I2C",&(i2c->mutex));

    if( (err = rt_mutex_release(&(i2c->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "I2C: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }
    
    if( res < 0 )
	return -EIO;		

    #ifdef DBG_LL_I2C
    util_pdbg(DBG_DEBG, "Written 0x%x on address:0x%x\n",address,arg1);
    #endif
    
    return 0;
}
