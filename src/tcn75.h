#ifndef __TCN75_H__
#define __TCN75_H__
// registers

#define TCN75_REG_TEMP   0x00 /* Temperature - 16bits */
#define TCN75_REG_CONFIG 0x01 /* Configuration - 8bits */
#define TCN75_REG_THYST  0x02 /* Hystereshis */
#define TCN75_REG_TSET   0x03 /* Temperature Set point */

/* CONFIG REGISTER */
#define TCN75_REG_CONFIG_SHUTDOWN_MASK  0x01 
#define TCN75_REG_CONFIG_COMPINT_MASK   0x02
#define TCN75_REG_CONFIG_POLARITY_MASK  0x04 
#define TCN75_REG_CONFIG_FAULTQUEU_MASK 0x18 
// #define TCN75_REG_CONFIG_ZERO_MASK      0xEO /* Should be Zero */
#define TCN75_REG_CONFIG_ON             0x00


int tcn75_init(uint8_t address, int i2cbus);
int tcn75_read_msb(uint8_t address, int i2cbus,int8_t* temp);
int tcn75_read(uint8_t address, int i2cbus,int16_t* temp);
int tcn75_shutdown(uint8_t address, int i2cbus);

#endif
