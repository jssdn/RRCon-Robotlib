#ifndef __ADCDEVICES_H__
#define __ADCDEVICES_H__

// Robot devices parameters 

#include "max1231adc.h"

// ADC Inintelligible defines for the platform 

/* IR Sensors */
#define ADC_GP1 MAX1231_CONV_AIN00 
#define ADC_GP2 MAX1231_CONV_AIN01
#define ADC_GP3 MAX1231_CONV_AIN02
#define ADC_GP4 MAX1231_CONV_AIN03
#define ADC_GP5 MAX1231_CONV_AIN04
#define ADC_GP6 MAX1231_CONV_AIN05
/* Gyros with their V for differential signaling (bigger noise rejection) */
#define ADC_GYROZ MAX1231_CONV_AIN06
#define ADC_VGYROZ MAX1231_CONV_AIN07
#define ADC_GYROY MAX1231_CONV_AIN08
#define ADC_VGYROY MAX1231_CONV_AIN09
#define ADC_GYROX MAX1231_CONV_AIN10
#define ADC_VGYROX MAX1231_CONV_AIN11
/* General purpose connector with Power + 4 ADC Inputs */
#define ADC_OTHERS1 MAX1231_CONV_AIN12
#define ADC_OTHERS2 MAX1231_CONV_AIN13
#define ADC_OTHERS3 MAX1231_CONV_AIN14 /* This can also be REF- */
#define ADC_OTHERS4 MAX1231_CONV_AIN15 


#endif
