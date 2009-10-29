#ifndef __LIS3LV02DL_H__
#define __LIS3LV02DL_H__

// registers
#define LIS3_REG_WHOAMI 0x0f /* Should return 0x3a */

#define LIS3_REG_OFFSETX 0x16 /* Digital offset trimming of X-Axis */
#define LIS3_REG_OFFSETY 0x17 /* Digital offset trimming of Y-Axis */
#define LIS3_REG_OFFSETZ 0x18 /* Digital offset trimming of Z-Axis */

#define LIS3_REG_GAINX 0x19 /* Digital gain trimming of X-Axis */
#define LIS3_REG_GAINY 0x1a /* Digital gain trimming of Y-Axis */
#define LIS3_REG_GAINZ 0x1b /* Digital gain trimming of Z-Axis */

#define LIS3_REG_CTRLREG1 0x20 
#define LIS3_REG_CTRLREG2 0x21
#define LIS3_REG_CTRLREG3 0x22

#define LIS3_REG_HPFILTERRESET 0x23

#define LIS3_REG_STATUSREG 0x27

#define LIS3_REG_OUTX_L 0x28
#define LIS3_REG_OUTX_H 0x29
#define LIS3_REG_OUTY_L 0x2a
#define LIS3_REG_OUTY_H 0x2b
#define LIS3_REG_OUTZ_L 0x2c
#define LIS3_REG_OUTZ_H 0x2d

#define LIS3_REG_WU_CFG 0x30
#define LIS3_REG_WU_SRC 0x31 
#define LIS3_REG_WU_ACK 0x32

#define LIS3_REG_WU_THS_L 0x34
#define LIS3_REG_WU_THS_H 0x35 
#define LIS3_REG_WU_THS_DURATION 0x36 

#define LIS3_REG_DD_CFG 0x38
#define LIS3_REG_DD_SRC 0x39
#define LIS3_REG_DD_ACK 0x3a

#define LIS3_REG_DD_THSI_L 0x3c
#define LIS3_REG_DD_THSI_H 0x3d
#define LIS3_REG_DD_THSE_L 0x3e 
#define LIS3_REG_DD_THSE_H 0x3f

/* Control register 1 */

#define LIS3_REG_CTRLREG1_PD_MASK  0xc0 /* Control reg1 Power down control */
#define LIS3_REG_CTRLREG1_DEC_MASK 0x30 /* Control reg1 Decimation factor control */
#define LIS3_REG_CTRLREG1_STEN_MASK  0x08 /* Control reg1 Self test enable */
#define LIS3_REG_CTRLREG1_ZEN_MASK 0x04 /* Control reg1 Z-axis enable */
#define LIS3_REG_CTRLREG1_YEN_MASK 0x02 /* Control reg1 Y-axis enable */
#define LIS3_REG_CTRLREG1_XEN_MASK 0x01 /* Control reg1 X-axis enable */

#define LIS3_REG_CTRLREG1_PD_ON   0xc0 /* Device On  */
#define LIS3_REG_CTRLREG1_PD_OFF  0x00 /* Power down  */

#define LIS3_REG_CTRLREG1_DEC_512 0x00 /* Decimation by 512 */
#define LIS3_REG_CTRLREG1_DEC_128 0x10 /* Decimation by 128 */
#define LIS3_REG_CTRLREG1_DEC_32  0x20 /* Decimation by 32 */
#define LIS3_REG_CTRLREG1_DEC_8   0x30 /* Decimation by 8 */

/* Control register 2 */

#define LIS3_ID 0x3a 

// The 1 bit enable registers are 1: On 0: Off

/* Control register 2 */

#define LIS3_REG_CTRLREG2_FS_MASK   0x80 /* Full scale 0:+-2g 1:+-6g */
#define LIS3_REG_CTRLREG2_BDU_MASK  0x40 /* Block data update 0:continous 1:not updated between LSB and MSB reading */
#define LIS3_REG_CTRLREG2_BLE_MASK  0x20 /* Big/Little ending 0:LE 1:BE */
#define LIS3_REG_CTRLREG2_BOOT_MASK 0x10 /* Reboot memory content */
#define LIS3_REG_CTRLREG2_IEN_MASK  0x08 /* Interrupt enable 0:data ready on RDY pad 1:interrupt on RDY pad */
#define LIS3_REG_CTRLREG2_DRDY_MASK 0x04 /* Enable data ready generation */
#define LIS3_REG_CTRLREG2_SIM_MASK  0x02 /* SPI mode 0: 4-wire 3:3-wire */
#define LIS3_REG_CTRLREG2_DAS_MASK  0x01 /* Data alignment 0:12 bit right justified 1:16 left justified */

#define LIS3_REG_CTRLREG2_FS_SET2G   0x00 
#define LIS3_REG_CTRLREG2_FS_SET6G   0x80 

/* Control register 3 */

#define LIS3_REG_CTRLREG3_ECK_MASK  0x80 /* External clock. Default 0. 0:internal  */
#define LIS3_REG_CTRLREG3_HPDD_MASK 0x40 /* High pass filter enabled for direction detection. Default 0. 0:bypassed 1: enabled */
#define LIS3_REG_CTRLREG3_HPFF_MASK 0x20 /* High pass filter enabled for free-fall and wake-up 0: bypassed 1:enabled */
#define LIS3_REG_CTRLREG3_FDS_MASK  0x10 /* Filtered data selection. 0: internal filter bypassed 1:enabled */
// 0xc0 is reserved 
#define LIS3_REG_CTRLREG3_CFS_MASK  0x03 /* High pass cut-off frequency selection */

// Fcutoff(-3db) = 0.318 * ODRx / (HPC * 2 ) 
#define LIS3_REG_CTRLREG3_CFS_HPC512    0x00 
#define LIS3_REG_CTRLREG3_CFS_HPC1024   0x01 
#define LIS3_REG_CTRLREG3_CFS_HPC2048   0x02 
#define LIS3_REG_CTRLREG3_CFS_HPC4096   0x03 

/* Status register */

#define LIS3_REG_STATUSREG_ZYXOR_MASK 0x80 /* Z,Y and X Data overrun */
#define LIS3_REG_STATUSREG_ZOVER_MASK 0x40 /* Z Data overrun */
#define LIS3_REG_STATUSREG_YOVER_MASK 0x20 /* Y Data overrun */
#define LIS3_REG_STATUSREG_XOVER_MASK 0x10 /* X Data overrun */
#define LIS3_REG_STATUSREG_ZYXDA_MASK 0x08 /* Z,Y and X Data available */ 
#define LIS3_REG_STATUSREG_ZDA_MASK   0x04 /* Z Data available */ 
#define LIS3_REG_STATUSREG_YDA_MASK   0x02 /* Y Data available */ 
#define LIS3_REG_STATUSREG_XDA_MASK   0x01 /* X Data available */ 

// Utils 
#define SCALE_FACTOR_6G_16bit 5211.3
#define SCALE_FACTOR_2G_16bit 16384.0

#define SCALE_FACTOR_6G_12bit 341.3
#define SCALE_FACTOR_2G_12bit 2048
//---------------------------------------------------------------------------------------------------------------------

#include <native/mutex.h> 
#include "i2ctools.h"

typedef struct{
    //Driver
    I2CDEV* i2c; 
    uint8_t address;
    RT_MUTEX mutex;
    //Data
    int16_t xacc; 
    int16_t yacc;
    int16_t zacc;
    //Calibrated data
    int16_t xcal;
    int16_t ycal;
    int16_t zcal;
    
    char data_overrun;
} LIS3LV02DL;

int lis3lv02dl_init(LIS3LV02DL* acc, I2CDEV* i2c, uint8_t address);

int lis3lv02dl_clean(LIS3LV02DL* acc);

int lis3lv02dl_id_check(LIS3LV02DL* acc);

int lis3lv02dl_poweroff(LIS3LV02DL* acc);

int lis3lv02dl_read(LIS3LV02DL* acc);

int lis3lv02dl_calib(LIS3LV02DL* acc);

int lis3lv02dl_init_3axis(LIS3LV02DL* acc);

#endif
