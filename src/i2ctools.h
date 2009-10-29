#ifndef __I2CTOOLS_H__
#define __I2CTOOLS_H__

#include <native/mutex.h>

typedef struct{    
    uint8_t i2cbus;    
    RT_MUTEX mutex;
    int file;
    char filename[80];
} I2CDEV; 

int i2c_init(I2CDEV* i2c, uint8_t bus);
int i2c_clean(I2CDEV* i2c);

/* SMBus I2C functions for read/write */
int i2c_get( I2CDEV* i2c, uint8_t address, uint8_t daddress, char csize);
int i2c_set( I2CDEV* i2c, uint8_t address, uint8_t daddress, char csize, unsigned value);

/* Special I2C functions for read/write with 3 parameters(see HMC6352) */
int i2c_get_3com( I2CDEV* i2c, uint8_t address, uint8_t arg1, uint8_t arg2);
int i2c_set_1com( I2CDEV* i2c, uint8_t address, uint8_t arg1);

#endif
