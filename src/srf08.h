#ifndef __SRF08_H__
#define __SRF08_H__

#include "i2ctools.h"
// REGS 

#define SRF08_REG_0 0x00
#define SRF08_REG_1 0x01
#define SRF08_REG_2 0x02
#define SRF08_REG_3 0x03
#define SRF08_REG_4 0x04
#define SRF08_REG_5 0x05
#define SRF08_REG_6 0x06
#define SRF08_REG_7 0x07
#define SRF08_REG_8 0x08
#define SRF08_REG_9 0x09
#define SRF08_REG_10 0x0a
#define SRF08_REG_11 0x0b
#define SRF08_REG_12 0x0c
#define SRF08_REG_13 0x0d
#define SRF08_REG_14 0x0e
#define SRF08_REG_15 0x0f
#define SRF08_REG_16 0x10
#define SRF08_REG_17 0x11
#define SRF08_REG_18 0x12
#define SRF08_REG_19 0x13
#define SRF08_REG_20 0x14
#define SRF08_REG_21 0x15
#define SRF08_REG_22 0x16
#define SRF08_REG_23 0x17
#define SRF08_REG_24 0x18
#define SRF08_REG_25 0x19
#define SRF08_REG_26 0x1a
#define SRF08_REG_27 0x1b
#define SRF08_REG_28 0x1c
#define SRF08_REG_29 0x1d
#define SRF08_REG_30 0x1e
#define SRF08_REG_31 0x1f
#define SRF08_REG_32 0x20
#define SRF08_REG_33 0x21
#define SRF08_REG_34 0x22
#define SRF08_REG_35 0x23

// ININTELLIGIBLE REGSsrf08_fire // function

#define SRF08_REG_CMD 0x00     /* Command register */
#define SRF08_REG_REV 0x00     /* Software revision register */

#define SRF08_REG_LIGHT   0x01 /* Light Sensor register */
#define SRF08_REG_MAXGAIN 0x01 /* Maximum gain register */

#define SRF08_REG_RANGE 0x02   /* Range register */
/* Echos */
#define SRF08_REG_1STECHO_HIGH 0x02  /* 1st echo high byte */
#define SRF08_REG_1STECHO_LOW 0x03   /* 1st echo high byte */

#define SRF08_REG_2NDECHO_HIGH 0x04  /* 1st echo high byte */
#define SRF08_REG_2NDECHO_LOW 0x05   /* 1st echo high byte */

#define SRF08_REG_3RDECHO_HIGH 0x06  /* 1st echo high byte */
#define SRF08_REG_3RDECHO_LOW 0x07   /* 1st echo high byte */
/* The rest of the echo regs go up to the 17th echo in pairs of two */
#define SRF08_REG_17THECHO_HIGH 0x22 /* 1th echo high byte */
#define SRF08_REG_17THECHO_LOW 0x23  /* 1th echo high byte */

//commands 

// RANGING MODE - IN CM, INCHES OR USECONDS
#define SRF08_CMD_RG_RESINCH  0x50
#define SRF08_CMD_RG_RESCM    0x51
#define SRF08_CMD_RG_RESUSEC  0x52

// ARTIFICIAL NEURAL NETWORK MODE
#define SRF08_CMD_ANN_RESINCH   0x53
#define SRF08_CMD_ANN_RESCM     0x54
#define SRF08_CMD_ANN_RESUSEC   0x55

// ADDRESS CHANGING CHANGING SEQUENCE 
#define SRF08_CMD_CHG_SEQ1   0xA0
#define SRF08_CMD_CHG_SEQ2   0xA5
#define SRF08_CMD_CHG_SEQ3   0xAA

// values 

#define SRF08_VAL_DEF_RANGE 0xff /* Default (max) range */
#define SRF08_VAL_DEF_GAIN  0x1f /* Default (max) analog gain */

#define SRF08_VAL_MAX_GAIN  0x1f /* Analog gain of 1025 */
#define SRF08_VAL_MIN_GAIN  0x00 /* Analog gain of 93 */

#define SRF08_VAL_MAX_ADDRESS  0x7F /* Max i2c address which the SRF08 can have */
#define SRF08_VAL_MIN_ADRESS   0x70 /* Min i2c address which the SRF08 can have */


#include <native/mutex.h>
#include "util.h"

#define srf08_sleep_max() \
        __usleep(65000)

typedef struct{
    I2CDEV* i2c; /* I2C device where the sensor is attached */
    uint8_t address; /* I2C bus address */
    RT_MUTEX mutex;  /* Mutex */
//     int16_t readings[17];    
} SRF08; 
    
int srf08_init(SRF08* sonar,I2CDEV* i2c, uint8_t address);

int srf08_clean(SRF08* sonar);

int srf08_get_echo(SRF08* sonar, uint8_t n);

int srf08_get_light(SRF08* sonar);

inline int srf08_fire_inch(SRF08* sonar);

inline int srf08_fire_cm(SRF08* sonar);

inline int srf08_fire_usec(SRF08* sonar);

#endif 
