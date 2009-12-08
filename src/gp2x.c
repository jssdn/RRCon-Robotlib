/**
    @file gp2x.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Conversion tables for IR analog rangers GP2x 
    
    @author Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
 
    @section LICENSE 
    
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
    
    @version 0.4-Xenomai
    @deprecated The should serve as an example as this is deprecated and hasn't been tested lately.     
    
*/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/types.h>

#include "max1231adc.h"
#include "gp2x.h"

// TODO: This could be improved with a precision experimentation lookup-table from Volts-Distance
// The following table is based on the specificacions provided from the manufacturer
// Distance vs deciVolts

// TODO: not very optimal search algorithm, but the LUT is small. 
int gp2dx_d120_v2cm(int dvolts)
{
    int i; 
    int tmp = 0;

    // Starts from the end to avoid going into the non-singular parts of the funcition
    for ( i = GP2D120_LUT_Y ; i > 0 ; i-- ) 
    {
        tmp = gp2d120_lut[i][1]; 
        if ( tmp >= dvolts){
            break; 
        }
    }

    return gp2d120_lut[i][0] ; 
}
