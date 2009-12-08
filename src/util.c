/**
    @file util.c 
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Auxiliary and debug functions 
    
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

*/

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "util.h"
#include "version.h" 

/**
* @brief Initialization for the I2C device
*
* @param msg Debug level: @see util.h
* @param fmt Formatted string 
*
* Wrapper for printf debugging with debug level support
*
*/

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

/**
* @brief Simple timestamp printing
*
* Timestamp printing through localtime()
*
*/

void print_timestamp()  
{  
     time_t ltime;     /* calendar time */  
     ltime=time(NULL); /* get current cal time */  
     printf("%s",asctime( localtime(&ltime) ) );  
}  

/**
* @brief Library banner printing
*
* Simple banner printing for versioning, authoring and release notes.
*
*/

void print_banner()
{
    util_pdbg(DBG_INFO, "------------------------------------------------------------\n");
    util_pdbg(DBG_INFO, "%s Version:%s\n",ROBOTLIB_NAME_STRING,ROBOTLIB_VERSION_STRING);
    util_pdbg(DBG_INFO, "Author:%s \n",ROBOTLIB_AUTHOR_STRING);
    util_pdbg(DBG_INFO, "Release notes:%s \n",ROBOTLIB_REVISION_STRING );
    util_pdbg(DBG_INFO, "------------------------------------------------------------\n");
}

/**
* @brief Sleep miliseconds
*
* @param t Time in ms
*
* Sleep in milisecond through nanosleep() call 
*
*/

void __msleep(long t)
{
	struct timespec ts = {
	.tv_sec = 0,
	.tv_nsec = t*1000000,
	};	
	nanosleep(&ts,NULL);
}

/**
* @brief Sleep microseconds
*
* @param t Time in us
*
* Sleep in microsecond through nanosleep() call 
*
*/

void __usleep(long long t)
{
	struct timespec ts = {
	.tv_sec = 0,
	.tv_nsec = t*1000,
	};	
	nanosleep(&ts,NULL);
}

/**
* @brief Sleep nanoseconds
*
* @param t Time in us
*
* Sleep in nanoseconds through nanosleep() call 
*
*/

void __nanosleep(long long t)
{
	struct timespec ts = {
	.tv_sec = 0,
	.tv_nsec = t,
	};	
	nanosleep(&ts,NULL);
    
}
