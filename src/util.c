/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: util.c Auxiliary and debug functions 
	        
    License: Licensed under GPL2.0 

    Copyright (C) Jorge Sánchez de Nova
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
#include <stdarg.h>
#include <time.h>
#include "util.h"
#include "version.h" 

// Hint for the compiler to optimize this function when not used 
#if DBG_LEVEL != 0
void util_pdbg(int msg, char *fmt, ...)
#else 
inline void util_pdbg(int msg, char *fmt, ...)
#endif 
{ 
  #if DBG_LEVEL != 0
  va_list args_ptr;
  static char buf[DBG_MSG_LENGTH];

  if ( msg <= DBG_LEVEL ) {  
      va_start(args_ptr,fmt);
      vsprintf(buf, fmt, args_ptr);
//       strcat(buf, "\n");
      va_end(args_ptr);
/*      printf("%s",buf);*/
      fprintf(stderr, buf);
      fflush(stderr);
    }
   #endif
}

void print_timestamp()  
{  
     time_t ltime;     /* calendar time */  
     ltime=time(NULL); /* get current cal time */  
     printf("%s",asctime( localtime(&ltime) ) );  
}  

void print_banner()
{
    util_pdbg(DBG_INFO, "------------------------------------------------------------\n");
    util_pdbg(DBG_INFO, "%s Version:%s\n",ROBOTLIB_NAME_STRING,ROBOTLIB_VERSION_STRING);
    util_pdbg(DBG_INFO, "Author:%s \n",ROBOTLIB_AUTHOR_STRING);
    util_pdbg(DBG_INFO, "Release notes:%s \n",ROBOTLIB_REVISION_STRING );
    util_pdbg(DBG_INFO, "------------------------------------------------------------\n");
}
