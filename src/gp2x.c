/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: gp2x.c Conversion tables for IR analog rangers GP2x 

    License: Licensed under GPL2.0 

*  ******************************************************************************* */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/types.h>

#include "max1231adc.h"
#include "gp2x.h"

// TODO: not very optimal search algorithm, but the LUT is small. 

int gp2dx_d120_v2cm(int dvolts)
{
    int i; 
    int tmp = 0;

    // Starts from the end to avoid going into the non-singular parts of the funcition
    for ( i = sizeof(gpd120_lut[][0]) ; i > 0 ; i-- ) 
    {
        tmp = gpd120_lut[i][1]; 
        if ( tmp >= dvolts){
            break; 
        }
    }

    return gpd120_lut[i][0] ; 
}
