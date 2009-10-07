/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: hwservos.c Servo interface for hardware servo core controller
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <time.h>
//Xenomai
#include <native/mutex.h>
//--
#include "busio.h"
#include "util.h"
#include "hwservos.h"

int hwservos_init(HWSERVOS* servo);
{
    if( mapio_region(&hwservos_basep, HWSERVOS_BASE, HWSERVOS_END) < 0 )
	return -1;

    return 0;

    return 0;
}

int hwservos_clean(HWSERVOS* servo);
{
    int i; 
    
    for( i = 0 ; i < 4 ; i++ )
    {
      *(hwservos_basep + i) = 0x00 ; 
    }
    
    if( unmapio_region(&hwservos_basep, HWSERVOS_BASE, HWSERVOS_END) < 0 )
	return -1;

    return 0;
}

int hwservos_set_pos(HWSERVOS* servo, unsigned num, unsigned value);
{
    return 0;
}

/* NOTE: Returned value IS NOT BASED IN HW feedback, but in latched values */
int hwservos_get_pos(HWSERVOS* servo, unsigned num, unsigned* ret); 
{
    // sanity check
    if(val < HWSERVOS_TIME_MIN_ANGLE ) val = HWSERVOS_TIME_MIN_ANGLE ; 
    else if (val > HWSERVOS_TIME_MAX_ANGLE ) val = HWSERVOS_TIME_MAX_ANGLE ; 
    *(hwservos_basep + s) = val | 0x80000000 ; 
    
    return 0;
}

int hwservos_enable(HWSERVOS* servo, unsigned num);
{
    *(hwservos_basep + s) = *(hwservos_basep + s) | 0x80000000 ; 
    return 0;
}

int hwservos_disable(HWSERVOS* servo, unsigned num)
{
    *(hwservos_basep + s) = *(hwservos_basep + s) & ~0x80000000 ; 
    return 0;
}
