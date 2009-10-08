/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
             Based in code from i2ctools-3.0.1 by Jean Delvare <khali@linux-fr.org>
                                                  Frodo Looijaard <frodol@dds.nl>
                                                  Mark D. Studebaker <mdsxyz123@yahoo.com>

    Code: i2ctools.c I2C Send/Receive functions

    License: Licensed under GPL2.0 

    NOTE: This code assumes Xilinx i2c-bitbang module is used . Hence, capabilities of byte/word reading are assumed and not checked
    
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
//#include <pthread.h> /* Previously programmed with pthread */

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
	util_pdbg(DBG_WARN, "I2C: File descriptor could not be allocated. Error %d\n", err);
	return -ENODEV; 
    }

    util_pdbg(DBG_INFO, "Succesfully open file in %s on bus %d\n",i2c->filename,i2c->i2cbus);

    // Mutex init
    if( (err = rt_mutex_create(&(i2c->mutex), NULL)) < 0 ){
	    util_pdbg(DBG_WARN, "I2C: Error rt_mutex_create: %d\n", err);
	    return err;
    } 
   
    return 0; 
}

int i2c_clean(I2CDEV* i2c)
{
    int err;

    util_pdbg(DBG_DEBG, "I2C: Cleaning I2C Device...\n");
      
    // delete mutex
    if( ( err = rt_mutex_delete(&(i2c->mutex)) ) < 0 ){
	util_pdbg(DBG_WARN, "I2C: Mutex cannot be deleted \n");
	return err; 
    }
    
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
    
    if( (err = rt_mutex_acquire(&(i2c->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "I2C: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
 
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
    
    if( (err = rt_mutex_release(&(i2c->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "I2C: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }

    if (res < 0) {
	    fprintf(stderr, "Error: Read failed\n");
	    return -EIO;
    }
        
    util_pdbg(DBG_DEBG, "0x%0*x\n", csize == 'w' ? 4 : 2, res);    
    
    return res;
}

int i2c_set(I2CDEV* i2c, uint8_t address, uint8_t daddress, char csize, unsigned value)
{
    int res,err;

    if (address > 0x7f) {
	util_pdbg(DBG_WARN , "I2C: Chip address invalid!\n");
	return -EADDRNOTAVAIL;
    }

    if( (err = rt_mutex_acquire(&(i2c->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "I2C: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
    
    set_slave_addr(i2c->file, address, 0);

    if (csize == 'w')
	res = i2c_smbus_write_word_data(i2c->file, daddress, value & 0xffff);
    else 
	res = i2c_smbus_write_byte_data(i2c->file, daddress, value & 0xff);
       
/*  TODO: REMOVE THIS? Readback? --> innefficient!
    if (size == I2C_SMBUS_WORD_DATA) {
	    res = i2c_smbus_read_word_data(file, daddress);
    } else {
	    res = i2c_smbus_read_byte_data(file, daddress);
    }*/
    
    if( (err = rt_mutex_release(&(i2c->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "I2C: Couldn't release mutex . Error : %d \n", err);
	return err;     
    }
    
    if (res < 0) {
	fprintf(stderr, "Error: Write failed\n");
	return -EIO;		
    }

    util_pdbg(DBG_DEBG, "Written 0x%x on daddress:0x%x\n",value, daddress);
    
    return 0; 
}

/* Special I2C functions for read/write with 3 parameters(see HMC6352) */
int i2c_get_3comp( I2CDEV* i2c, uint8_t address, uint8_t arg1, uint8_t arg2)
{
    int err,res;	        

    if( (err = rt_mutex_acquire(&(i2c->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "I2C: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }
 
    // Write 3 arguments on the i2c bus
    i2c_set(i2c,address,arg1,'b', arg2);    
    set_slave_addr(i2c->file, address, 0);      
    res = i2c_smbus_read_byte(i2c->file);

    if( (err = rt_mutex_release(&(i2c->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "I2C: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }
    
    return 0;
}

int i2cset_1comp( I2CDEV* i2c, uint8_t address, uint8_t arg1)
{
    int res,err;

    if (address > 0x7f) {
	util_pdbg(DBG_WARN , "I2C: Chip address invalid!\n");
	return -EADDRNOTAVAIL;
    }

    if( (err = rt_mutex_acquire(&(i2c->mutex), TM_INFINITE)) < 0){  // block until mutex is released
	util_pdbg(DBG_WARN, "I2C: Couldn't acquire mutex . Error : %d \n", err);	
	return err;
    }

    set_slave_addr(i2c->file, address, 0);
    res = i2c_smbus_write_byte(i2c->file,arg1);
    
    if( (err = rt_mutex_release(&(i2c->mutex))) < 0 ){
	util_pdbg(DBG_WARN, "I2C: Couldn't release mutex . Error : %d \n", err);
	return err; 
    }
    
    if( res < 0 )
	return -EIO;		

    util_pdbg(DBG_DEBG, "Writing 0x%x on address:0x%x\n",address,arg1);
    
    return 0;
}
