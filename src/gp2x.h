#ifndef __GP2DX_H__
#define __GP2DX_H__

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/types.h>

#include "max1231adc.h"
#include "gp2x.h"

// GP2D120 

// Not singular values from 0 until X
#define GP2D120_NON_SINGULAR_UNTIL 2

// TODO: This could be improved with a precision experimentation lookup-table from Volts-Distance
// The following table is based on the specificacions provided from the manufacturer
// Distance vs deciVolts

static int gp2d120_lut[][] = {  { 0 , 0  }, 
                                { 1 , 19 },
                                { 2 , 22 },
                                { 3 , 30 },
                                { 4 , 27 },
                                { 5 , 23 },
                                { 6 , 20 },
                                { 7 , 18 },
                                { 8 , 16 },
                                { 9 , 14 },
                                { 10 , 13 },
                                { 11 , 12 },
                                { 12 , 11 },
                                { 13 , 10 },
                                { 14 , 9 },
                                { 15 , 8 },
                                { 16 , 8 },
                                { 17 , 8 },
                                { 18 , 7 },
                                { 19 , 7 },
                                { 20 , 7 },
                                { 21 , 6 },
                                { 22 , 6 },
                                { 23 , 6 },
                                { 24 , 6 },
                                { 25 , 5 },
                                { 26 , 5 },
                                { 27 , 5 },
                                { 28 , 5 },
                                { 29 , 5 },
                                { 30 , 4 },
                                { 31 , 4 },
                                { 32 , 4 },
                                { 33 , 4 },
                                { 34 , 4 },
                                { 35 , 4 },
                                { 36 , 4 },
                                { 37 , 4 },
                                { 38 , 3 },
                                { 39 , 3 },
                                { 40 , 3 } };

//TODO: LUTs for the other gp2 sensors
int gp2dx_d120_v2cm(int volts);

#endif