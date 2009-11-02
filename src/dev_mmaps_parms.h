#ifndef __GPIOMAPS_H__
#define __GPIOMAPS_H__

/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: Description for IO mappings and parameters in the ML403-based development platform

    License: Licensed under GPL2.0 

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

// TODO: Get them in an automated manner
  
/** GPIOs **/

/* GPIO for General Outputs */
/* Several devices are attached(muxed) to the following outputs -
   4 LEDs (4bit), 5 Position LEDS (5bit) and a USB_RESET_SIGNAL(1bit) */

#define GENERAL_OUTPUTS_BASE        0x814a0000
#define GENERAL_OUTPUTS_END         0x814affff
#define GENERAL_OUTPUTS_NUM_OF_CHAN 1
#define GENERAL_OUTPUTS_NUM_OF_GPIO 17

#define GENERAL_OUTPUTS_LED4_MASK    0x0000f
#define GENERAL_OUTPUTS_LED4_SHIFT   0
#define GENERAL_OUTPUTS_LEDPOS_MASK  0x001f0
#define GENERAL_OUTPUTS_LEDPOS_SHIFT 4
//#define GENERAL_USB_RESET_MASK    0x00200 /* Not for use */

/* GPIO for General Inputs */

/* Several devices are attached(muxed) to the following inputs - 
   Directional buttons (5bit), Bumpers(4bits), an Accelerator interrupt line (1bit) and an ADC End Of Conversion signal (1bit) */

#define GENERAL_INPUTS_BASE         0x81420000
#define GENERAL_INPUTS_END          0x8142ffff
#define GENERAL_INPUTS_NUM_OF_CHAN  1
#define GENERAL_INPUTS_NUM_OF_GPIO  10
#define GENERAL_INPUTS_PUSHBUT_MASK 0x0001f
#define GENERAL_INPUTS_PUSHBUT_SHIFT 0
#define GENERAL_INPUTS_BUMPERS_MASK 0x001E0
#define GENERAL_INPUTS_BUMPERS_SHIFT 5
// #define GENERAL_INPUTS_ACC_RDY_MASK 0x00200
// #define GENERAL_INPUTS_ACC_RDY_SHIFT 9
// #define GENERAL_INPUTS_ADC_EOC_MASK 0x00400
// #define GENERAL_INPUTS_ADC_EOC_SHIFT 10

#define GENERAL_INPUTS_IRQ_NO 	7  
#define GENERAL_INPUTS_IRQ_PRIO 1	  

/* Output IO for LCD 16x2 */
#define LCD16X2_BASE        0x81460000
#define LCD16X2_END         0x8146ffff
#define LCD16X2_NUM_OF_CHAN 1
#define LCD16X2_NUM_OF_GPIO 7
#define LCD16X2_LCD_MASK    0x0007f

/* 8 Bit GPIO */
#define FPGA_GPIO8_BASE        0x81480000
#define FPGA_GPIO8_END         0x8148ffff
#define FPGA_GPIO8_NUM_OF_CHAN 1
#define FPGA_GPIO8_NUM_OF_GPIO 8
#define FPGA_GPIO8_MASK	       0x00ff
#define FPGA_GPIO8_IRQ_NO      8
#define FPGA_GPIO8_IRQ_PRIO    1

/** Motor devices **/

/* PWM module */
#define MOTORS_BASE 0xca400000
#define MOTORS_END  0xca40ffff
#define MOTORS_MAX_NUM_OF_CORES 16

#define MOTORS_NUM_OF 4
#define MOTORS_MAX_SPEED 1023   /* 11 bits signed  - PWM Duty Cycle */
#define MOTORS_MAX_FREQ_DIV 255 /* 8 bits unsigned - Frequency divider over BUS Frequency */

/* Quadrature decoders */
#define QENC_BASE 0xc4600000
#define QENC_END  0xc460ffff
#define QENC_MAX_NUM_OF_CORES 16

#define QENC_NUM_OF 4

/* PID Core address */
//TODO: Yet to be implemented
//#define PID_BASE 0x75020000
//#define PID_END  0x7502ffff

/** Servos **/
#define HWSERVOS_BASE 0xc5600000
#define HWSERVOS_END  0xc560FFFF
#define HWSERVOS_NUM_OF 4
#define HWSERVOS_MAX_NUM_OF 8

/* Standard values */
#define HWSERVOS_TIME_MAX_ANGLE 2000 /* 2 ms to reach the maximum aperture (typ 2-2.2ms) */
#define HWSERVOS_TIME_MIN_ANGLE 1000 /* 1 ms to reach the minimum aperture (typ 0.8-1ms) */
#define HWSERVOS_TIME_MID_ANGLE 1500 /* 1.5 ms to reach the minimum aperture (typ 0.8-1ms) */

/* Values for servos with extended control */
// #define HWSERVOS_TIME_MAX_ANGLE 2200 /* 2 ms to reach the maximum aperture (typ 2-2.2ms) */
// #define HWSERVOS_TIME_MIN_ANGLE 800 /* 1 ms to reach the minimum aperture (typ 0.8-1ms) */
// #define HWSERVOS_TIME_MID_ANGLE 1500 /* 1.5 ms to reach the minimum aperture (typ 0.8-1ms) */

#define HWSERVOS_EN_MASK 0x80000000 

/** I2C and devices attached to I2C **/

/* I2C Devices */
#define I2C_BUS_0 "/dev/i2c-0"
#define I2C_BUS_1 "/dev/i2c-1"
//#define I2C_BUS_2 "/dev/i2c-2"

/* Attached to I2C */

//FIXED IN THE BOARD
#define I2C_ACCELEROMETER_BUS 0
#define I2C_ACCELEROMETER_ADDRESS 0x1d

/* Variable, but one to agree where to place them  */
#define I2C_MAGNETICCOMPASS_BUS 1
#define I2C_MAGNETICCOMPASS_ADDRESS 0x21 

#define I2C_SONAR0_BUS 0
#define I2C_SONAR0_ADDRESS 0x70

#define I2C_SONAR1_BUS 1
#define I2C_SONAR1_ADDRESS 0x70

/* Debug */
#ifdef DEBUGALL 
    #define DBG_LEVEL 5
#else 
    #ifdef DEBUGWARN
        #define DBG_LEVEL 3
    #endif
#endif

/* Default: Debug all */    
#ifndef DBG_LEVEL
    #define DBG_LEVEL 5 
#endif

/* Uncomment the following for low level debugging in the communication devices */
//#define DBG_LL_I2C /* Debug Low-level IO calls from the I2C device */
//#define DBG_LL_SPI /* Debug Low-level IO calls from the SPI device */

#endif 
