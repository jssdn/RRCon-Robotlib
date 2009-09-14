#ifndef __I2CDEVICES_H__
#define __I2CDEVICES_H__

// Robot devices parameters 

#define I2C_BUS_0 "/dev/i2c-0"
#define I2C_BUS_1 "/dev/i2c-1"
//#define I2C_BUS_2 "/dev/i2c-2"

// I2C Inintelligible defines for the platform 

//FIXED IN THE BOARD
#define I2C_ACCELEROMETER_BUS 0
#define I2C_ACCELEROMETER_ADDRESS 0x1d

#define I2C_TEMPSENSOR_BUS 1
#define I2C_TEMPSENSOR_ADDRESS 0x48

//VARIABLE BUT WE NEED TO AGREE HERE WE PLACE THEM 
#define I2C_MAGNETICCOMPASS_BUS 1
#define I2C_MAGNETICCOMPASS_ADDRESS 0x21 

#define I2C_SONAR0_BUS 0
#define I2C_SONAR0_ADDRESS 0x70

//TODO: CHANGE THE ADDRESS OF THE SONAR 
#define I2C_SONAR1_BUS 0
#define I2C_SONAR1_ADDRESS 0x72 

#endif
