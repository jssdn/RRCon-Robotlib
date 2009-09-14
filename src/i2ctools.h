#ifndef __I2CTOOLS_H__
#define __I2CTOOLS_H__

//errors TODO: CHANGE TO USE ERRORNO
#define EBUSNINVALID    1
#define ECADDINVALID    2
#define EDADDINVALID    3
#define EMODEINVALID    4
#define EDATAOUTOFRANGE 5
#define EOPENFILEFAIL   6
#define EREADFAIL       7
#define EWRITEFAIL      8

//For HMC6352 special i2c calls
int i2cget_3comp( uint8_t address, uint8_t arg1, uint8_t arg2, uint8_t i2cbus,  pthread_mutex_t* i2cmutex);
int i2cset_1comp( uint8_t address, uint8_t arg1, uint8_t i2cbus,  pthread_mutex_t* i2cmutex);

int i2cget_3com( uint8_t address, uint8_t arg1, uint8_t arg2, uint8_t i2cbus);
int i2cset_1com( uint8_t address, uint8_t arg1, uint8_t i2cbus);

/*
IMPORTANT NOTE: THESE FUNCTIONS ARE NOT THREAD SAFE! IF YOU WANT TO DO MULTITHREAD, YOU SHOULD TAKE CARE OF THE MUTUAL EXCLUSION WHEN COMMUNICATING USING I2CGET/I2CSET JUST AS PREVIOUS FUNCTIONS OR OTHERWISE A DIFFERENT PROCESS MIGHT INTERFERE. 
*/

// For typical SMBus
// i2cget
int i2cgetp( uint8_t address, uint8_t daddress, uint8_t i2cbus, char csize, pthread_mutex_t* i2cmutex);

int i2csetp( uint8_t address, uint8_t daddress, int value, uint8_t i2cbus, char csize, pthread_mutex_t* i2cmutex);

int i2cget( uint8_t address, uint8_t daddress, uint8_t i2cbus, char csize);
// i2cset
int i2cset( uint8_t address, uint8_t daddress, int value, uint8_t i2cbus, char csize);

#endif
