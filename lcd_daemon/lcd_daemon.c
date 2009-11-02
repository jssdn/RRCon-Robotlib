/** ******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author: Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: Simple LCD daemon that allows system message printing through pipes

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

* ******************************************************************************* **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <fcntl.h>

//Xenomai
#include <native/task.h>
#include <native/timer.h>
#include <native/pipe.h>
//-- 

#include "lcd.h"            /* LCD */

/* Xenomai task variables */
#define STACK_SIZE 8192
#define LCD_PRIO 30

/* Platform devices */
LCD lcd; 

/* Xenomai per-task variables */
RT_TASK lcd_ptr;
RT_PIPE lcd_pipe_desc;

/* Xenomai Periodic tasks times (See scale below) */
//			               --s-ms-us-ns
RTIME lcd_period_ns =  		 0500000000llu;

/* Other global variables */
int dbool = 0; 

/* Dirt log functions */
void slog(char *fmt, ...)
{ 
    va_list args_ptr;
    static char buf[80];
    va_start(args_ptr,fmt);
    vsprintf(buf, fmt, args_ptr);
    va_end(args_ptr);
    if (dbool)
	syslog(LOG_INFO, "%s", buf);    
    else{
	printf("%s",buf);
	fflush(NULL);
    }
}

/* 
*  lcd_task : Create a pipe called "LCD_PIPE" on /dev/rtp7 and reads from it
*  Other processes in the system can write to it to print messages on the LCD 
*
*/
void lcd_task(void *cookie) {
    int err;
    ssize_t numb; 
    char buf[80];   
    
    /* LCD initialization */
    if( ( err = lcd_init(&lcd)) < 0 ){
	slog("LCD: Errror initializing lcd...\n"); 
	return; 
    }
    
    lcd_on(&lcd);
    lcd_clear(&lcd);
    lcd_print(&lcd, "RRCon Platform  HW:3.1 SW:0.4  ");
    
    /* Connect the kernel-side of the message pipe to the special device file /dev/rtp7. */
    if( (err = rt_pipe_create(&lcd_pipe_desc,"LCD_PIPE",7,0)) < 0 ){
	slog("LCD: Error creating pipe. Exiting... %d\n",err);
	perror(NULL);
	return;
    }

    /* We block until any process write into the pipe and just print the message on the LCD */
    while (1) {		
	if ((numb = rt_pipe_read(&lcd_pipe_desc, (void*)buf, 80, TM_INFINITE )) < 0) {
	    slog("LCD: Error reading from pipe. Exiting... %d\n",err);
	    perror(NULL);
	    return;
	}
	slog("LCD: Received: %s\n", buf ); 
	lcd_print(&lcd, buf);	
    }
}

void clean_exit()
{
    printf("\n...cleanup!\n");
    rt_pipe_delete(&lcd_pipe_desc);
    rt_task_delete(&lcd_ptr);    
    if(dbool)
	closelog();
    fflush(NULL);
    exit(0);
}

int main( int argc, char** argv )
{
    int err; 
    pid_t pid, sid;

    if( argc == 2 ){	
	if( strcmp(argv[1],"-nd" ) == 0)
	    dbool = 0; 
	else if( strcmp(argv[1],"-d" ) == 0)
	    dbool = 1; 
	    else{ 
		printf("Usage:\tlcd_deamon -d ( Daemonize )\n\tlcd_daemon -nd ( No fork )\n");
		fflush(NULL);
		exit(0);
	    }
    }else{
	printf("Too many/few arguments!\n");
	printf("Usage:\tlcd_deamon -d ( Daemonize )\n\tlcd_daemon -nd ( No fork )\n");
	fflush(NULL);
	exit(0);
    }
    
    // Assure a clean exit
    signal(SIGTERM, clean_exit);	
    signal(SIGINT, clean_exit);	   
    
    // Avoid page faults	
    if( ( err = mlockall(MCL_CURRENT | MCL_FUTURE)) < 0 ) {
	    printf("MAIN: Memory could not be locked. Exiting...\n");
	    exit(-1);
    }   
    
    if (dbool){
	pid = fork();
        if (pid < 0) {
		exit(-1);
        }
	/* Kill father */
        if (pid > 0) {
		exit(0);
        }
	
	/* Avoid page fault for the forked process too */
	if( ( err = mlockall(MCL_CURRENT | MCL_FUTURE)) < 0 ) {
		printf("MAIN: Memory could not be locked. Exiting...\n");
		exit(-1);
	}   

        /* Change the file mode mask */
        umask(0);
                
        /* Open any logs here */        
	openlog("lcd_log", LOG_PID|LOG_CONS, LOG_USER);
	syslog(LOG_INFO, "Initalizating LCD syslog\n");

        /* Create a new SID for the child process */
        if ((sid = setsid()) < 0) {
                /* Log the failure */
                exit(-1);
        }
        
        
        /* Change the current working directory */
        if ((chdir("/")) < 0) {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }
        
        /* Close out the standard file descriptors */

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
	if( (err = rt_task_spawn(&lcd_ptr, "LCD Task", STACK_SIZE, LCD_PRIO, 0, &lcd_task, NULL)) < 0){
		syslog(LOG_ERR, "MAIN: LCD task could not be correctly initialized\n");
		exit(err);
	}	
                
        while (1) {
           /* Do Nothing... */           
           sleep(3000);
        }
	
    } else {
	if( (err = rt_task_spawn(&lcd_ptr, "LCD Task", STACK_SIZE, LCD_PRIO, 0, &lcd_task, NULL)) < 0){
		printf("MAIN: LCD task could not be correctly initialized\n");
		exit(err);
	}	
	
	// wait for signal & return of signal handler
	pause();
	fflush(NULL);	
    }
    
    return 0; 
}

