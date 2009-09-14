#ifndef __HMC6352_H__
#define __HMC6352_H__

// Commands
#define HMC6352_CMD_WRITE_EEPROM  0x77
#define HMC6352_CMD_READ_EEPROM   0x72
#define HMC6352_CMD_WRITE_RAM     0x47
#define HMC6352_CMD_READ_RAM      0x67
#define HMC6352_CMD_SLEEP         0x53
#define HMC6352_CMD_WAKEUP        0x57
#define HMC6352_CMD_UPBRIDGE      0x4f
#define HMC6352_CMD_ENTER_CALIB   0x43
#define HMC6352_CMD_EXIT_CALIB    0x45 
#define HMC6352_CMD_SAVEOP_EEPROM 0x4c
#define HMC6352_CMD_GETDATA       0x41

//EEPROM REGS 
#define HMC6352_EE_REG_ADDRESS  0x00 /* I2C Slave Address - 0x42 */
#define HMC6352_EE_REG_XOFF_MSB 0x01 /* Magnetometer X Offset MSB */
#define HMC6352_EE_REG_XOFF_LSB 0x02 /* Magnetometer X Offset LSB */
#define HMC6352_EE_REG_YOFF_MSB 0x03 /* Magnetometer Y Offset MSB */
#define HMC6352_EE_REG_YOFF_LSB 0x04 /* Magnetometer Y Offset LSB */
#define HMC6352_EE_REG_DELAY    0x05 /* Time Delay (0 – 255 ms) (default: 1) */
#define HMC6352_EE_REG_MEASURES 0x06 /* Number of Summed measurements(1-16) (default: 0x04)*/
#define HMC6352_EE_REG_VERSION  0x07 /* Software Version Number*/
#define HMC6352_EE_REG_OPMODE   0x08 /* Operation Mode Byte   (default: 0x50) */

// ID
#define HMC6352_ID 0x42

// RAM REGS
#define HMC6352_RAM_REG_OUTMODE   0x4e /* Output data mode */
#define HMC6352_RAM_REG_OPMODE    0x74 /* Operational mode - Shadowed in EEPROM 0x08 */

// OPERATION MODE CONFIG

// The rest should be set to 0
#define HMC6352_REG_OPMODE_FREQ_MASK  0x60 /* Frequency selection in continuous mode */
#define HMC6352_REG_OPMODE_RESET_MASK 0x10 /* Periodic Set/Reset ON/OFF*/
#define HMC6352_REG_OPMODE_OP_MASK    0x03 /* Operation mode */

#define HMC6352_REG_OPMODE_FREQ_1HZ   0x00
#define HMC6352_REG_OPMODE_FREQ_5HZ   0x20
#define HMC6352_REG_OPMODE_FREQ_10HZ  0x40
#define HMC6352_REG_OPMODE_FREQ_20HZ  0x60

#define HMC6352_REG_OPMODE_RESET_ON  0x10
#define HMC6352_REG_OPMODE_RESET_OFF 0x00

#define HMC6352_REG_OPMODE_OP_STANDBY   0x00
#define HMC6352_REG_OPMODE_OP_QUERY     0x01 
#define HMC6352_REG_OPMODE_OP_CONTINOUS 0x02

// OUT MODES
#define HMC6352_RAM_REG_OUTMODE_MASK    0x07
#define HMC6352_RAM_REG_OUTMODE_HEADING 0x00
#define HMC6352_RAM_REG_OUTMODE_RAWX    0x01
#define HMC6352_RAM_REG_OUTMODE_RAWy    0x03 
#define HMC6352_RAM_REG_OUTMODE_X       0x03
#define HMC6352_RAM_REG_OUTMODE_Y       0x04

// struct hmc6352struct{
// };
// 
// typedef struct hmc6352struct hmc6352s; 

int hmc6532_idcheck(uint8_t address, int i2cbus);

int hmc6532_init_standby(uint8_t address, int i2cbus);

int hmc6532_init_query(uint8_t address, int i2cbus);

int hmc6532_init_continous(uint8_t address, int i2cbus,uint8_t freq);

int hmc6532_read_nowait(uint8_t address, int i2cbus, uint16_t* degrees);

int hmc6532_read_wait(uint8_t address, int i2cbus, uint16_t* degrees);

//TODO: The ones below need testing
int hmc6532_enter_calibration(uint8_t address, int i2cbus);

int hmc6532_exit_calibration(uint8_t address, int i2cbus);

int hmc6532_sleep(uint8_t address, int i2cbus);

int hmc6532_wakeup(uint8_t address, int i2cbus);

// TODO: Measure summing, time delay, software version
// int hmc6532_read_calibrate(uint8_t address, int i2cbus);
// 
// int hmc6532_get_heading(uint8_t address, int i2cbus, int16_t* degrees);
// 
// int hmc6532_get_xy_raw(uint8_t address, int i2cbus, int16_t* x, int16_t* x );
// 
// int hmc6532_get_xy(uint8_t address, int i2cbus, int16_t* x, int16_t* x );
// 
// int hmc6532_saveop_to_eeprom(uint8_t address, int i2cbus);

#endif

