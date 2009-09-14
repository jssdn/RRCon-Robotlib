
/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
             Based in code from i2ctools-3.0.1 by Jean Delvare <khali@linux-fr.org>
                                                  Frodo Looijaard <frodol@dds.nl>
                                                  Mark D. Studebaker <mdsxyz123@yahoo.com>

    Code: i2ctools.c I2C Send/Receive functions

    License: Licensed under GPL2.0 

    Notes: This code assumes Xilinx i2c-bitbang module is used . Hence, capabilities of byte/word reading are assumed and not checked

*  ******************************************************************************* */

#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "i2cbusses.h"
#include <fcntl.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <pthread.h> 

#include "i2ctools.h"
//TODO: Unify csize 

// this is for the hmc6352 memory read mode with 3 arguments system -> address, command , memory address 
// and subsequent read
int i2cget_3comp( uint8_t address, uint8_t arg1, uint8_t arg2, uint8_t i2cbus, pthread_mutex_t* i2cmutex)
{
    int file, res;	
    char filename[20];

    if(i2cmutex != 0) // thread safe
      if( (res = pthread_mutex_lock(i2cmutex)))
      {
	#ifdef DEBUGMODE
	printf("Error acquiring mutex for i2c connection\n");
	#endif 
	return res; 
      }

    // Write 3 arguments on the i2c bus
    if(( res = i2cset(address,arg1,arg2,i2cbus,'b')) < 0 ) // Checks for valid address is included
        return res; 

    if ((file = open_i2c_dev(i2cbus, filename, 0)) < 0 || (set_slave_addr(file, address, 0) < 0)) {
            return -EOPENFILEFAIL;
    }

    #ifdef DEBUGMODE
    printf("Succesfully open file in %s on bus %d\n",filename,i2cbus);
    #endif 

    // Direct read without daddress assignment 
    res = i2c_smbus_read_byte(file);

    close(file); 
    if(i2cmutex != 0) // thread safe
      pthread_mutex_unlock(i2cmutex);

    return 0;
}

int i2cset_1comp( uint8_t address, uint8_t arg1, uint8_t i2cbus, pthread_mutex_t* i2cmutex)
{
	int res, file;	
	char filename[20];

	if (i2cbus > 0x3f) {
            fprintf(stderr, "Error: I2CBUS argument invalid!\n");
            return -EBUSNINVALID;
	}

	if (address > 0x7f) {
            fprintf(stderr, "Error: Chip address invalid!\n");
            return -ECADDINVALID; 
	}

	if(i2cmutex != 0) // thread safe
	    if( res = pthread_mutex_lock(i2cmutex))
	    {
	      #ifdef DEBUGMODE
	      printf("Error acquiring mutex for i2c connection\n");
	      #endif 
	      return res; 
	    }

	if ((file = open_i2c_dev(i2cbus, filename, 0)) < 0 || (set_slave_addr(file, address, 0) < 0)) {
		return -EOPENFILEFAIL;
	}

        #ifdef DEBUGMODE
        printf("Writing 0x%x on address:0x%x\n",address,arg1);
        #endif 

        if( (res = i2c_smbus_write_byte(file,arg1)) < 0 )
	   return -EWRITEFAIL;		
	
	close(file);

	if(i2cmutex != 0) // thread safe
	  pthread_mutex_unlock(i2cmutex);

	return 0;
}

// SMBus based i2c functions 
int i2cgetp( uint8_t address, uint8_t daddress, uint8_t i2cbus, char csize, pthread_mutex_t* i2cmutex)
{
	int res, file;	
	char filename[20];
        int size;

        if (i2cbus > 0x3f) {
            fprintf(stderr, "Error: I2CBUS argument invalid!\n");
            return -EBUSNINVALID;
	}
	
	if ( address < 3 || address > 0x77) {
            fprintf(stderr, "Error: Chip address invalid!\n");
            return -ECADDINVALID;
	}
	
        switch (csize) 
        {
            case 'b': 
                size = I2C_SMBUS_BYTE_DATA; 
                break;
            case 'w': 
                size = I2C_SMBUS_WORD_DATA; 
                break;
            case 'c': 
                size = I2C_SMBUS_BYTE; 
                break;
            default:
                fprintf(stderr, "Error: Invalid mode!\n");
                return -EMODEINVALID;
	}

	if(i2cmutex != 0) // thread safe
	    if( res = pthread_mutex_lock(i2cmutex))
	    {
	      #ifdef DEBUGMODE
	      printf("Error acquiring mutex for i2c connection\n");
	      #endif 
	      return res; 
	    }

	file = open_i2c_dev(i2cbus, filename, 0);
	
	if (file < 0 || set_slave_addr(file, address, 0))
		return -EOPENFILEFAIL;

	switch (size)
	{
            case I2C_SMBUS_BYTE:
/*                    if (daddress >= 0) {
                            res = i2c_smbus_write_byte(file, daddress);
                            if (res < 0)
                                    fprintf(stderr, "Warning - write failed\n");
                    }
                    res = i2c_smbus_read_byte(file);*/
                    res = i2c_smbus_read_byte_data(file, daddress);
                    break;
            case I2C_SMBUS_WORD_DATA:
                    res = i2c_smbus_read_word_data(file, daddress);
                    break;
            default: /* I2C_SMBUS_BYTE_DATA */
                    res = i2c_smbus_read_byte_data(file, daddress);
	}
	
	close(file);

	if (res < 0) {
		fprintf(stderr, "Error: Read failed\n");
		return -EREADFAIL;
	}

        #ifdef DEBUGMODE
	printf("0x%0*x\n", size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);
        #endif
      
	close(file);

	if(i2cmutex != 0) // thread safe
	  pthread_mutex_unlock(i2cmutex);

	return res;
}

int i2csetp( uint8_t address, uint8_t daddress, int value, uint8_t i2cbus, char csize, pthread_mutex_t* i2cmutex)
{
	int res, size, file;	
	char filename[20];


	if (i2cbus > 0x3f) {
            fprintf(stderr, "Error: I2CBUS argument invalid!\n");
            return -EBUSNINVALID;
	}

	if (address > 0x7f) {
            fprintf(stderr, "Error: Chip address invalid!\n");
            return -ECADDINVALID; 
	}

        switch (csize) 
        {
            case 'b': 
                size = I2C_SMBUS_BYTE_DATA; 
                break;
            case 'w': 
                size = I2C_SMBUS_WORD_DATA; 
                break;
            default:
                fprintf(stderr, "Error: Invalid mode!\n");
                return -EMODEINVALID;
	}

	if (value < 0 || (size == I2C_SMBUS_BYTE_DATA && value > 0xff) || (size == I2C_SMBUS_WORD_DATA && value > 0xffff)) {
		fprintf(stderr, "Error: Data value out of range!\n");
		return -EDATAOUTOFRANGE;
	}

	if(i2cmutex != 0) // thread safe
	    if( res = pthread_mutex_lock(i2cmutex))
	    {
	      #ifdef DEBUGMODE
	      printf("Error acquiring mutex for i2c connection\n");
	      #endif 
	      return res; 
	    }
	
	if ((file = open_i2c_dev(i2cbus, filename, 0)) < 0 || (set_slave_addr(file, address, 0) < 0)) {
		return -EOPENFILEFAIL;
	}

        #ifdef DEBUGMODE
        printf("Succesfully open file in %s on bus %d\n",filename,i2cbus);
        printf("Writing 0x%x on daddress:0x%x\n",value, daddress);
        #endif 

	if (size == I2C_SMBUS_WORD_DATA) {
		res = i2c_smbus_write_word_data(file, daddress, value);
	} else {
		res = i2c_smbus_write_byte_data(file, daddress, value);
	}
	
	if (res < 0) {
		fprintf(stderr, "Error: Write failed\n");
		close(file);
		return -EWRITEFAIL;		
	}

	if (size == I2C_SMBUS_WORD_DATA) {
		res = i2c_smbus_read_word_data(file, daddress);
	} else {
		res = i2c_smbus_read_byte_data(file, daddress);
	}
	
	close(file);

        #ifdef DEBUGMODE
/*	if (res < 0) {
		printf("Warning - readback failed\n");
	} else if (res != value) {
		printf("Warning - data mismatch - wrote "
		       "0x%0*x, read back 0x%0*x\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value,
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);
	} else {
		printf("Value 0x%0*x written, readback matched\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value);
	}*/
        #endif

	close(file); 

	if(i2cmutex != 0) // thread safe
	  pthread_mutex_unlock(i2cmutex);

	
	return 0; 
}


int i2cget_3com( uint8_t address, uint8_t arg1, uint8_t arg2, uint8_t i2cbus)
{
    int file, res;	
    char filename[20];


    // Write 3 arguments on the i2c bus
    if(( res = i2cset(address,arg1,arg2,i2cbus,'b')) < 0 ) // Checks for valid address is included
        return res; 

    if ((file = open_i2c_dev(i2cbus, filename, 0)) < 0 || (set_slave_addr(file, address, 0) < 0)) {
            return -EOPENFILEFAIL;
    }

    #ifdef DEBUGMODE
    printf("Succesfully open file in %s on bus %d\n",filename,i2cbus);
    #endif 

    // Direct read without daddress assignment 
    res = i2c_smbus_read_byte(file);

    close(file); 
    return 0;
}

int i2cset_1com( uint8_t address, uint8_t arg1, uint8_t i2cbus)
{
	int res, file;	
	char filename[20];

	if (i2cbus > 0x3f) {
            fprintf(stderr, "Error: I2CBUS argument invalid!\n");
            return -EBUSNINVALID;
	}

	if (address > 0x7f) {
            fprintf(stderr, "Error: Chip address invalid!\n");
            return -ECADDINVALID; 
	}


	if ((file = open_i2c_dev(i2cbus, filename, 0)) < 0 || (set_slave_addr(file, address, 0) < 0)) {
		return -EOPENFILEFAIL;
	}

        #ifdef DEBUGMODE
        printf("Writing 0x%x on address:0x%x\n",address,arg1);
        #endif 

        if( (res = i2c_smbus_write_byte(file,arg1)) < 0 )
	   return -EWRITEFAIL;		
	
	close(file);

	return 0;
}

// SMBus based i2c functions 
int i2cget( uint8_t address, uint8_t daddress, uint8_t i2cbus, char csize)
{
	int res, file;	
	char filename[20];
        int size;

        if (i2cbus > 0x3f) {
            fprintf(stderr, "Error: I2CBUS argument invalid!\n");
            return -EBUSNINVALID;
	}
	
	if ( address < 3 || address > 0x77) {
            fprintf(stderr, "Error: Chip address invalid!\n");
            return -ECADDINVALID;
	}
	
        switch (csize) 
        {
            case 'b': 
                size = I2C_SMBUS_BYTE_DATA; 
                break;
            case 'w': 
                size = I2C_SMBUS_WORD_DATA; 
                break;
            case 'c': 
                size = I2C_SMBUS_BYTE; 
                break;
            default:
                fprintf(stderr, "Error: Invalid mode!\n");
                return -EMODEINVALID;
	}

	file = open_i2c_dev(i2cbus, filename, 0);
	
	if (file < 0 || set_slave_addr(file, address, 0))
		return -EOPENFILEFAIL;

	switch (size)
	{
            case I2C_SMBUS_BYTE:
/*                    if (daddress >= 0) {
                            res = i2c_smbus_write_byte(file, daddress);
                            if (res < 0)
                                    fprintf(stderr, "Warning - write failed\n");
                    }
                    res = i2c_smbus_read_byte(file);*/
                    res = i2c_smbus_read_byte_data(file, daddress);
                    break;
            case I2C_SMBUS_WORD_DATA:
                    res = i2c_smbus_read_word_data(file, daddress);
                    break;
            default: /* I2C_SMBUS_BYTE_DATA */
                    res = i2c_smbus_read_byte_data(file, daddress);
	}
	
	close(file);

	if (res < 0) {
		fprintf(stderr, "Error: Read failed\n");
		return -EREADFAIL;
	}

        #ifdef DEBUGMODE
	printf("0x%0*x\n", size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);
        #endif
      
	close(file);

	return res;
}

int i2cset( uint8_t address, uint8_t daddress, int value, uint8_t i2cbus, char csize)
{
	int res, size, file;	
	char filename[20];


	if (i2cbus > 0x3f) {
            fprintf(stderr, "Error: I2CBUS argument invalid!\n");
            return -EBUSNINVALID;
	}

	if (address > 0x7f) {
            fprintf(stderr, "Error: Chip address invalid!\n");
            return -ECADDINVALID; 
	}

        switch (csize) 
        {
            case 'b': 
                size = I2C_SMBUS_BYTE_DATA; 
                break;
            case 'w': 
                size = I2C_SMBUS_WORD_DATA; 
                break;
            default:
                fprintf(stderr, "Error: Invalid mode!\n");
                return -EMODEINVALID;
	}

	if (value < 0 || (size == I2C_SMBUS_BYTE_DATA && value > 0xff) || (size == I2C_SMBUS_WORD_DATA && value > 0xffff)) {
		fprintf(stderr, "Error: Data value out of range!\n");
		return -EDATAOUTOFRANGE;
	}

	if ((file = open_i2c_dev(i2cbus, filename, 0)) < 0 || (set_slave_addr(file, address, 0) < 0)) {
		return -EOPENFILEFAIL;
	}

        #ifdef DEBUGMODE
        printf("Succesfully open file in %s on bus %d\n",filename,i2cbus);
        printf("Writing 0x%x on daddress:0x%x\n",value, daddress);
        #endif 

	if (size == I2C_SMBUS_WORD_DATA) {
		res = i2c_smbus_write_word_data(file, daddress, value);
	} else {
		res = i2c_smbus_write_byte_data(file, daddress, value);
	}
	
	if (res < 0) {
		fprintf(stderr, "Error: Write failed\n");
		close(file);
		return -EWRITEFAIL;		
	}

	if (size == I2C_SMBUS_WORD_DATA) {
		res = i2c_smbus_read_word_data(file, daddress);
	} else {
		res = i2c_smbus_read_byte_data(file, daddress);
	}
	
	close(file);

        #ifdef DEBUGMODE
/*	if (res < 0) {
		printf("Warning - readback failed\n");
	} else if (res != value) {
		printf("Warning - data mismatch - wrote "
		       "0x%0*x, read back 0x%0*x\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value,
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, res);
	} else {
		printf("Value 0x%0*x written, readback matched\n",
		       size == I2C_SMBUS_WORD_DATA ? 4 : 2, value);
	}*/
        #endif

	close(file); 
	
	return 0; 
}
