/**
    @file max1231adc.h
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief [HEADER] Driver for MAX1231/1230 ADCs
*/

#ifndef __MAX1231_ADC_H__
#define __MAX1231_ADC_H__

#include <xspidev.h>

// TODO: DEFINE A LOOKUP-TABLE FOR DELAY TIMES ( FASTER ) 

// MAX1231 Conversion register
// 1 x x x x x x x
#define MAX1231_CONV                          	     0x80
// Power-on state: 1000 0000
#define MAX1231_CONV_POR                      	     0x80

// Channel Selection
#define MAX1231_CONV_AIN00                           0x80     /*!  10000xxx AIN0 */
#define MAX1231_CONV_AIN01                           0x88     /*!  10001xxx AIN1 */
#define MAX1231_CONV_AIN02                           0x90     /*!  10010xxx AIN2 */
#define MAX1231_CONV_AIN03                           0x98     /*!  10011xxx AIN3 */
#define MAX1231_CONV_AIN04                           0xA0     /*!  10100xxx AIN4 */
#define MAX1231_CONV_AIN05                           0xA8     /*!  10101xxx AIN5 */
#define MAX1231_CONV_AIN06                           0xB0     /*!  10110xxx AIN6 */
#define MAX1231_CONV_AIN07                           0xB8     /*!  10111xxx AIN7 */
#define MAX1231_CONV_AIN08                           0xC0     /*!  11000xxx AIN8 */
#define MAX1231_CONV_AIN09                           0xC8     /*!  11001xxx AIN9 */
#define MAX1231_CONV_AIN10                           0xD0     /*!  11010xxx AIN10 */
#define MAX1231_CONV_AIN11                           0xD8     /*!  11011xxx AIN11 */
#define MAX1231_CONV_AIN12                           0xE0     /*!  11100xxx AIN12 */
#define MAX1231_CONV_AIN13                           0xE8     /*!  11101xxx AIN13 */
#define MAX1231_CONV_AIN14                           0xF0     /*!  11110xxx AIN14 */
#define MAX1231_CONV_AIN15                           0xF8     /*!  11111xxx AIN15 */

// Actions
#define MAX1231_CONV_SCAN_00_N                               0x80     /*! 1xxxx000  Scan 0,1,2,...N */
#define MAX1231_CONV_SCAN_T_00_N                             0x81     /*! 1xxxx001  Scan T,0,1,2,..N */
#define MAX1231_CONV_SCAN_N_15                               0x82     /*! 1xxxx010  Scan N,N+1,...,15 */
#define MAX1231_CONV_SCAN_T_N_15                             0x83     /*! 1xxxx011  Scan T,N,N+1,...,15 */
#define MAX1231_CONV_SINGLE_REPEAT                           0x84     /*! 1xxxx10x  Read repeatedly */
#define MAX1231_CONV_SINGLE_READ                             0x86     /*! 1xxxx11x  Read once */
#define MAX1231_CONV_TEMP                                    0x81     /*! 1xxxxxx1  Read temp */
//
#define MAX1231_ACTION_MASK                                  0x87     /*! 1xxxx111 bits to test*/

//--------------------------------------------------
// MAX1231 Setup register
// 0 1 x x x x 0 0
//
// Setup register may optionally be followed by
// one of the the differential configuration registers.
// 01xxxx10 followed by a second byte, selecting Unipolar-Differential inputs
// 01xxxx11 followed by a second byte, selecting Bipolar-Differential inputs
#define MAX1231_SETUP                                   0x40      /*! 01xxxx00 no additional bytes */
#define MAX1231_SETUP_UNIDIFF                           0x42      /*! 01xxxx10 followed by another byte */
#define MAX1231_SETUP_BIPDIFF                           0x43      /*! 01xxxx11 followed by another byte */
//
// Power-on state: 0110 0000
#define MAX1231_SETUP_POR                       0x60
//
// Clock Mode
// 0100xxxx pin16=CNVST, Int clock, Triggered by CNVST pulse
// 0101xxxx pin16=CNVST, Int clock, Triggered by CNVST pulses, custom Tacq
// 0110xxxx pin16=AIN15, Int clock, Triggered by conversion register write
// 0111xxxx pin16=AIN15, Ext clock, Triggered by conversion register write
#define MAX1231_SETUP_INTCLK_CNVST                                  0x40      /*! 0100xxxx CNVST */
#define MAX1231_SETUP_INTCLK_CNVST_TACQ                             0x50      /*! 0101xxxx CNVST */
#define MAX1231_SETUP_INTCLK                                        0x60      /*! 0110xxxx AIN15 */
#define MAX1231_SETUP_EXTCLK                                        0x70      /*! 0111xxxx AIN15 */
//
// Reference Voltage
// 01xx00xx pin15=AIN14, Internal reference, need wake-up delay
// 01xx01xx pin15=AIN14, External reference, single-ended
// 01xx10xx pin15=AIN14, Internal reference, always on
// 01xx11xx pin15=REF-, External reference, differential
#define MAX1231_SETUP_INTREF_SLEEP                           0x40      /*!  01xx00xx AIN14 */
#define MAX1231_SETUP_EXTREF                                 0x44      /*!  01xx01xx AIN14 */
#define MAX1231_SETUP_INTREF_ACTIVE                          0x48      /*!  01xx10xx AIN14 */
#define MAX1231_SETUP_EXTREF_DIFF                            0x4C      /*!  01xx11xx REF(-) */


// MAX1231 Unipolar-Differential input pairs
// Byte Following MAX1231_SETUP_UNIDIFF
// 0 1 x x x x 1 0 unidiff
//
// Power-on state: 0110 0010 0000 0000
#define MAX1231_SETUP_UNIDIF_POR                          0x00
//
#define MAX1231_SETUP_UNIDIF0001                          0x80
#define MAX1231_SETUP_UNIDIF0203                          0x40
#define MAX1231_SETUP_UNIDIF0405                          0x20
#define MAX1231_SETUP_UNIDIF0607                          0x10
#define MAX1231_SETUP_UNIDIF0809                          0x08
#define MAX1231_SETUP_UNIDIF1011                          0x04
#define MAX1231_SETUP_UNIDIF1213                          0x02
#define MAX1231_SETUP_UNIDIF1415                          0x01
//
// MAX1231 Bipolar-Differential input pairs
// Byte Following MAX1231_SETUP_BIPDIFF
// 0 1 x x x x 1 1 bipdiff
//
// Power-on state: 0110 0011 0000 0000
#define MAX1231_SETUP_BIPDIF_POR                          0x00
//
#define MAX1231_SETUP_BIPDIF0001                          0x80
#define MAX1231_SETUP_BIPDIF0203                          0x40
#define MAX1231_SETUP_BIPDIF0405                          0x20
#define MAX1231_SETUP_BIPDIF0607                          0x10
#define MAX1231_SETUP_BIPDIF0809                          0x08
#define MAX1231_SETUP_BIPDIF1011                          0x04
#define MAX1231_SETUP_BIPDIF1213                          0x02
#define MAX1231_SETUP_BIPDIF1415                          0x01
//--------------------------------------------------
// MAX1231 Averaging register
// 0 0 1 x x x x x
//
// Power-on state: 0010 0000
#define MAX1231_AVERAGE_POR                          0x20
//
// Averaging
// 001000xx One measurement result (no averaging)
// 001100xx Mean of 4 measurement results
// 001101xx Mean of 8 measurement results
// 001110xx Mean of 16 measurement results
// 001111xx Mean of 32 measurement results
#define MAX1231_AVERAGE_1                          0x20    /*!  001000xx   No averaging */
#define MAX1231_AVERAGE_4                          0x30    /*!  001100xx   Mean of 4 measurements */
#define MAX1231_AVERAGE_8                          0x34    /*!  001101xx   Mean of 8 measurements */
#define MAX1231_AVERAGE_16                         0x38    /*!  001110xx   Mean of 16 measurements */
#define MAX1231_AVERAGE_32                         0x3C    /*!  001111xx   Mean of 32 measurements */

//
// Repeat Count
// Enabled by MAX1231_CONV_SINGLE_REPEAT 1xxxx10x
// Internal clock modes only
#define MAX1231_REPEAT_4                           0x20 /*! 001xxx00 4 times */
#define MAX1231_REPEAT_8                           0x21 /*! 001xxx01 8 times */
#define MAX1231_REPEAT_12                          0x22 /*! 001xxx10 12 times */
#define MAX1231_REPEAT_16                          0x23 /*! 001xxx11 16 times */
//--------------------------------------------------
// MAX1231 Reset register (reset command)
// 0 0 0 1 x x x x
//
// Reset only the FIFO entries
#define MAX1231_RESET_FIFO                         0x18 /*! 00011xxx Reset FIFO data regis*/
//
// Reset all registers to their power-on default states
#define MAX1231_RESET_ALL                          0x10 /*! 00010xxx Reset All Registers */

#define MAX1231_DELAY_TEMP                           62   /*! temp + reference up (us) */

#define MAX1231_CONF_UNIDIFF_MASK 0x01
#define MAX1231_CONF_BIPDIFF_MASK 0x02 

#include <native/mutex.h>
#include "xspidev.h"

typedef struct{
  XSPIDEV* xspi; ///< SPI device where the max1231 is connected to
  RT_MUTEX mutex; ///< Xenomai Mutex
//   uint8_t dest[34]; // 2*NumOfChannels + 2(temp)
  uint8_t pairs[8];  ///< CH0/1 - CH2/3 - CH4/5 - CH6/7 - CH8/9 - CH10/11 - CH12/13 - CH14/15
  uint8_t clock; ///< Clock and reference configuration
} MAX1231; 

int max1231_init(MAX1231* adc, XSPIDEV* spi);

int max1231_clean(MAX1231* adc);

/* Config ADC inputs in different ways according to 'conf' */
//NOTE: The caller should assure that the device is not accessed externally through a mutex/other somewhere else. Mutual exclusion is just guaranteed over the same xspidev structure.
int max1231_config(MAX1231* adc);

/* Low level write for 8bits to support genral commands*/
int adc_ll_write8(MAX1231* adc, uint8_t tx, int sleep);

/* Reads an array of 'len' bytes to 'dest_array' */
int adc_read(MAX1231* adc,uint8_t convbyte,uint8_t* dest_array, int len);

/* Read one two bytes measure from the ADC */
int adc_read_one_once(MAX1231* adc, uint8_t n, int* ret);

/* Reads in Scan mode from byte 0 to N */
int adc_read_scan_0_N(MAX1231* adc, uint8_t* dest, uint8_t n);

/* Simplify reading of temperature and returns value in degrees */
// Needs to divide by 8 afterwards
int adc_get_temperature(MAX1231* adc, int* ret);

/* TODO: Take it out from here */
#define CMD_ALL_SINGLE_TX 0x64
#define CMD_ALL_DIFF_TX 0x64

#define adc_config_all_uni_single(xspi) adc_ll_write8(xspi,MAX1231_SETUP_EXTREF | MAX1231_SETUP_INTCLK, 1000)
/* Config a pair('first', second)  in differential mode */
// #define adc_config_diff(xspi) 		adc_ll_write8(xspi,CMD_ALL_DIFF, CMD_ALL_DIFF)

/* Reset the ADC */
#define adc_reset(adc_ptr) 		adc_ll_write8(adc_ptr,MAX1231_RESET_ALL, 1000)
/* Reset FIFO */
#define adc_reset_fifo(adc_ptr) 		adc_ll_write8(adc_ptr,MAX1231_RESET_FIFO, 100)

#endif
