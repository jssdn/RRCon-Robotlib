/** ******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author: Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: Example code for library testing

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
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

//Xenomai
#include <native/task.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <native/intr.h>
//-- 

#include "gpio.h"            /* GPIO */
#include "platform_io.h"     /* High-level functions for GPIO devices */
#include "util.h"	     /* Some commonly used functions all across the library */
#include "motors.h"	     /* DC Motors and Encoders */
#include "hwservos.h"        /* RC Servos controller */
#include "i2ctools.h"	     /* I2C */
#include "xspidev.h"	     /* SPI */
#include "max1231adc.h"	     /* ADC */
#include "srf08.h"	     /* Sonar */
#include "lis3lv02dl.h"      /* Accelerometer */

/* Xenomai task variables */

#define STACK_SIZE 8192
#define WATCHDOG_PRIO 26
#define MAX_PRIO 1
#define ADC_PRIO 2
#define STD_PRIO 25

/* Global variables */
int irq_counter = 0; 
int end = 0; 
#define DEVSPI "/dev/spi0"
//#define DEVSPI "/dev/spidev32766.0"

/* Platform devices */
HWSERVOS servos; 
MOTOR motors;
XSPIDEV spi; 
MAX1231 adc; 
I2CDEV i2c1; 
I2CDEV i2c2; 
LIS3LV02DL acc; 
SRF08 srf08; 

/* Xenomai per-task variables */
RT_TASK watchdog_ptr;
RT_TASK main_task_ptr;
RT_TASK hwservos_ptr;
RT_TASK motors_ptr;
RT_TASK adc_ptr;
RT_TASK acc_ptr;
RT_TASK sonar_ptr;
RT_TASK clean_ptr;

/* Xenomai Periodic tasks times (See scale below) */
//			               --s-ms-us-ns
RTIME watchdog_period_ns =  		 2500000000llu;
RTIME hwservos_period_ns =  		 1000000000llu;
RTIME motors_period_ns =  		 3000000000llu;
RTIME adc_period_ns =  			 1000000000llu;
RTIME acc_period_ns =  			 3000000000llu;
RTIME sonar_period_ns = 		 1500000000llu;

/* ISR for GPIO - NOTE: Currently not being used */
void gpio_isr(void* cookie)
{	
    int err;
    RT_INTR* intr_desc = (RT_INTR*)cookie; 
    printf("IRQ: IRQ spawned. Waiting...\n");
    
    while(!end){
	if( ( err = rt_intr_wait(intr_desc, TM_INFINITE)) > 0){
	    if(++irq_counter == 10)
		    end = 1; //finish!
	}
	printf("IRQ: - IRQ counter %d\n",irq_counter);
	rt_intr_enable(intr_desc);
    }
}

/* Example of a Watchdog task */
void watchdog(void *cookie) {
    int err;
    unsigned but; 
/*	int k = 0xf8f; */
    int i = 0; 
    unsigned long overrun;
    pio_write_go_all(0,1);	    
    
    if ((err = rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(watchdog_period_ns))) < 0) {
	util_pdbg(DBG_WARN, "WATCHDOG: - Error while set periodic, code %d\n",err);
	return;
    }
    
    while (!end) {
	err = rt_task_wait_period(&overrun);
	if (err) {
	    util_pdbg(DBG_WARN,"WATCHDOG: Error while rt_task_wait_period %d\n",err);
	    return;
	}
	if( (err = pio_read_buttons(&but))< 0)
	    printf("WATCHDOG: error reading buttons\n");
	
	printf("WATCHDOG: - IRQ counter %d - Leds:0x%x But:0x%x\n",irq_counter,0x01<<i,but);
	
// 	pio_write_ledspos(i);
// 	pio_write_leds4(i);

// 		pio_write_go_all(k<<i);
	
// 	pio_write_go_all(0xfff,0);
	
	pio_write_go_all(0x01<<i,0);
	
	fflush(NULL);
// 	
	if(i == 10)
	    i = 0;
	else
	    i++;

/*	if(i == 0x1f)
	    i = 0;
	else
	    i++;*/
    }
}

/* Task that controls RC servos */
void hwservos_task(void* cookie)
{
    int err,i; 
    unsigned long overrun;
    
    unsigned angle = HWSERVOS_TIME_MIN_ANGLE;       
	    
    if ((err = rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(hwservos_period_ns))) < 0) {
	util_pdbg(DBG_WARN, "HWSERVOS_TASK: - Error while set periodic, code %d\n",err);
	return;
    }
    
    while (!end) {
	err = rt_task_wait_period(&overrun);
	
	if (err) {
	    util_pdbg(DBG_CRIT,"HWSERVOS_TASK: Error while rt_task_wait_period %d\n",err);
	    return;
	}
	
	if( angle > HWSERVOS_TIME_MAX_ANGLE )
 	    angle = HWSERVOS_TIME_MIN_ANGLE;	    
	else 
	    angle = (angle + 100);
	
	for( i = 0 ; i < HWSERVOS_NUM_OF ; i++)    
	    hwservos_set_pos(&servos, i, angle);        	
    }
}

/* Task that controls DC Motors and prints the status of the encoders */
void motors_task(void* cookie)
{
    int err, i; 
    unsigned long overrun;
    
    unsigned speed = 0;            
    int enc,enc_prev; 
	    
    if ((err = rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(motors_period_ns))) < 0) {
	util_pdbg(DBG_WARN, "MOTORS_TASK: - Error while set periodic, code %d\n",err);
	return;
    }
    
    /* Set Frequency divider for the PWM carrier in each motor*/
    for ( i = 0 ; i < MOTORS_NUM_OF ; i++)
	motors_pwm_set_freq_div(&motors, i, 3);	
    
    /* Set Speed in each motor */
    for ( i = 0 ; i < MOTORS_NUM_OF ; i++)
	    motors_pwm_set_speed(&motors, i, speed);	
    
    /* Reset encoder counters */
    for ( i = 0 ; i < QENC_NUM_OF ; i++)
	motors_qenc_setzero(&motors, i);
    
    while (!end) {
	err = rt_task_wait_period(&overrun);
	
	if (err) {
	    util_pdbg(DBG_CRIT,"MOTORS_TASK: Error while rt_task_wait_period %d\n",err);
	    return;
	}
	
	speed = (speed + 100)%MOTORS_MAX_SPEED;			
	
	for ( i = 0 ; i < MOTORS_NUM_OF ; i++)
	    motors_pwm_set_speed(&motors, i, speed);	
	
	for ( i = 0 ; i < QENC_NUM_OF ; i++)
	{
	    motors_qenc_read_enc(&motors, i, &enc, &enc_prev);
	    printf("MOTORS_TASK: - Quad Encoder %d counter: %d\n",i, enc);
	}
	
    }
    
}

/* Task that samples the ADC and give readings from channels */
void adc_task(void* cookie)
{
    int err,i,j; 
    unsigned long overrun;
    int ret; 
    
    uint8_t dest[32];    
        
    if ((err = rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(adc_period_ns))) < 0) {
	util_pdbg(DBG_WARN, "ADC_TASK: - Error while set periodic, code %d\n",err);
	return;
    }

    // Configuration with Gyros in differential mode
    /*
    adcconf.pairs[4] = MAX1231_CONF_BIPDIFF_MASK ;  // Gyro in differential mode
    adcconf.pairs[5] = MAX1231_CONF_BIPDIFF_MASK ;  // Gyro in differential mode
    adcconf.pairs[6] = MAX1231_CONF_BIPDIFF_MASK ;  // Gyro in differential mode
    adcconf.clock = 0x64 ; 
    adc_config(&xspi, &adcconf );
    */
   
     // Configuration with Gyros in unipolar mode
    adc_reset(&adc); 
    __usleep(10000); // TODO:needed? 
    adc_config_all_uni_single(&adc);
    __usleep(10000); // TODO:needed?

    while (!end) {
	err = rt_task_wait_period(&overrun);
	
	if (err) {
	    util_pdbg(DBG_CRIT,"ADC_TASK: Error while rt_task_wait_period %d\n",err);
	    return;
	}	
	//ADC ACTIONS
	adc_read_scan_0_N(&adc, dest, 15);	
	printf("ADC_TASK: Readings from all channels:\n");
	for( i = 0 ; i < 16 ; i++ )
	{
	    for( j = 0 ; j < 4 ; j++,i++ ){	   
		printf("\tChannel[%d]:%d",i, (int)( (int)dest[i*2+1] | ((int)dest[i*2]<<8)) );
	    }
	    printf("\n");
	}
	
	adc_get_temperature(&adc, &ret);
	printf("ADC_TASK: Temp:%.2f\n",(float)ret/8);
	adc_reset_fifo(&adc); // TODO:needed?
    }       
}

/* Task that samples the accelerometer, calibrates it and displays G measures */
void acc_task(void* cookie)
{
    int err; 
    unsigned long overrun;
    float x,y,z;
    	    
    if ((err = rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(motors_period_ns))) < 0) {
	util_pdbg(DBG_WARN, "ACC_TASK: - Error while set periodic, code %d\n",err);
 	return;
    }
    
    if( (err = lis3lv02dl_calib(&acc)) < 0 ){
         util_pdbg(DBG_WARN, "ACC_TASK: Error calibrating accelerometer!. Error: %d\n", err);
 	 return;
    }
           
    while (!end) {
	err = rt_task_wait_period(&overrun);
	
	if (err) {
	    util_pdbg(DBG_CRIT,"ACC_TASK: Error while rt_task_wait_period %d\n",err);
	    return;
	}
	//ACC actions	
	lis3lv02dl_read(&acc);
	
        x = (double)(acc.xacc - acc.xcal)/(SCALE_FACTOR_6G_16bit); 
        y = (double)(acc.yacc - acc.ycal)/(SCALE_FACTOR_6G_16bit); 
        z = (double)(acc.zacc - acc.zcal)/(SCALE_FACTOR_6G_16bit); 

        printf("ACC_TASK: X = %f\tY = %f\tZ = %f\n", x,y,z);
    }
}

/* Task that fires the Sonar and displays some echoes */
void sonar_task(void* cookie)
{
    int err, i; 
    unsigned long overrun;
    	    
    if ((err = rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(motors_period_ns))) < 0) {
	util_pdbg(DBG_WARN, "SONAR_TASK: - Error while set periodic, code %d\n",err);
 	return;
    }   
           
    while (!end) {
	err = rt_task_wait_period(&overrun);
	
	if (err) {
	    util_pdbg(DBG_CRIT,"SONAR_TASK: Error while rt_task_wait_period %d\n",err);
	    return;
	}
	//Sonar actions	
	srf08_fire_cm(&srf08);	
	srf08_sleep_max();

	for( i = 0; i < 5 ; i++ )
	    printf( "SONAR_TASK: ECHO:%d cm\n",srf08_get_echo(&srf08,i)); 
	}
}

/* RT Task that inits the devices and launches the other tasks */
void main_task(void* cookie)
{
    int err; 
    
    util_pdbg(DBG_INFO, "Initializing MOTORS/ENCODERS\n");
    		      
//     if( (err = pio_init_all(gpio_isr, NULL)) < 0 ) {
    if( (err = pio_init_all(NULL, NULL)) < 0 ) {
	util_pdbg(DBG_CRIT, "GPIO devices could not be correctly initialized\n");	    
	perror(NULL);
	exit(err);
    }
    
    util_pdbg(DBG_INFO, "Initializing HWServos\n");
       
    if( (err = hwservos_init(&servos, HWSERVOS_BASE,HWSERVOS_END, HWSERVOS_NUM_OF)) < 0 ) {
	util_pdbg(DBG_CRIT, "HWSERVOS devices could not be correctly initialized\n");	    
	perror(NULL);
	exit(err);
    }

    if( (err = rt_task_spawn(&hwservos_ptr, "HWServos", STACK_SIZE, STD_PRIO, 0, &hwservos_task, NULL)) < 0){
	util_pdbg(DBG_CRIT, "HWServos periodic task could not be correctly initialized\n");
	perror(NULL);
	exit(err);
    }
    
    util_pdbg(DBG_INFO, "Initializing Motors\n");
    
    if( (err = motors_init_motor(&motors, 
		      MOTORS_BASE, MOTORS_END, MOTORS_NUM_OF, 
		      QENC_BASE, QENC_END, QENC_NUM_OF, 
		      0, 0, 0)) < 0 ){
	util_pdbg(DBG_CRIT, "MOTOR/ENCODERS could not be correctly initialized\n");	    
	perror(NULL);
	exit(err);	
    }

    if( (err = rt_task_spawn(&motors_ptr, "Motors", STACK_SIZE, STD_PRIO - 1, 0, &motors_task, NULL)) < 0){
	util_pdbg(DBG_CRIT, "MOTORS/ENCODERS periodic task could not be correctly initialized\n");
	perror(NULL);
	exit(err);
    }
    
    util_pdbg(DBG_INFO, "Initializing SPI\n");
    
    //0s here set default options for the device
    if( (err = spi_init(&spi, DEVSPI, 0, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0)) < 0 ){
	util_pdbg(DBG_CRIT, "SPI could not be configured\n");
	perror(NULL);
	exit(err);
    }
    
    util_pdbg(DBG_INFO, "Initializing ADC\n");

    if( (err = max1231_init(&adc, &spi)) < 0 ){
	util_pdbg(DBG_CRIT, "ADC could not be initialized\n");
	perror(NULL);
	exit(err);
    }
    
    max1231_config(&adc);
    
    if( (err = rt_task_spawn(&adc_ptr, "ADC", STACK_SIZE, ADC_PRIO, 0, &adc_task, NULL)) < 0){
	    util_pdbg(DBG_CRIT, "ADC periodic task could not be correctly initialized\n");
	    perror(NULL);
	    exit(err);
    }    
    
    util_pdbg(DBG_INFO, "Initializing I2C\n");    
    
    if( (err = i2c_init(&i2c1,0)) < 0 ){
	util_pdbg(DBG_CRIT, "I2C-0 could not be initialized\n");
	perror(NULL);
	exit(err);
    }
        
    if( (err = i2c_init(&i2c2,1)) < 0 ){
	util_pdbg(DBG_CRIT, "I2C-1 could not be initialized\n");
	perror(NULL);
	exit(err);
    }
    
    util_pdbg(DBG_INFO, "Initializing Accelerometer\n");    
    
    if( (err = lis3lv02dl_init(&acc, &i2c1, I2C_ACCELEROMETER_ADDRESS)) < 0 ){
	util_pdbg(DBG_CRIT, "Accelerometer could not be initialized\n");
	perror(NULL);
	exit(err);
    }
    
    if( (err = lis3lv02dl_init_3axis(&acc)) < 0 ){
	util_pdbg(DBG_CRIT, "Accelerometer could not be configured\n");
	perror(NULL);
	exit(err);
    }

    if( (err = rt_task_spawn(&acc_ptr, "ACC", STACK_SIZE, STD_PRIO, 0, &acc_task, NULL)) < 0){
	    util_pdbg(DBG_CRIT, "ACC periodic task could not be correctly initialized\n");
	    perror(NULL);
	    exit(err);
    }    

    util_pdbg(DBG_INFO, "Initializing Sonar\n");    

    if( (err = srf08_init(&srf08, &i2c2, I2C_SONAR0_ADDRESS)) < 0 ){
	util_pdbg(DBG_CRIT, "Sonar could not be initialized\n");
	perror(NULL);
	exit(err);
    }

    if( (err = rt_task_spawn(&sonar_ptr, "Sonar", STACK_SIZE, STD_PRIO, 0, &sonar_task, NULL)) < 0){
	    util_pdbg(DBG_CRIT, "Sonar periodic task could not be correctly initialized\n");
	    perror(NULL);
	    exit(err);
    }
    
}

/* RT task to clean objects ( some cannot be cleaned from a non-rt space ) */
void clean_rt_task(void* cookie)
{
    //gpios
    pio_clean_all();
    //servos
    hwservos_clean(&servos);
    rt_task_delete(&hwservos_ptr);    
    //motors
    motors_clean_motor(&motors);    
    rt_task_delete(&motors_ptr);
    //spi
    max1231_clean(&adc);
    spi_clean(&spi);    
    rt_task_delete(&adc_ptr);
    //i2c 
    i2c_clean(&i2c1);
    i2c_clean(&i2c2);
    //Accelerometer 
    lis3lv02dl_clean(&acc);
    rt_task_delete(&acc_ptr);
    //Sonar
    srf08_clean(&srf08);
    rt_task_delete(&sonar_ptr);
    //watchdog
    rt_task_delete(&watchdog_ptr);
    //main
    rt_task_delete(&main_task_ptr);    
    
    end = 1;    
}

/* Signal-handler, to ensure clean exit on Ctrl-C */
void clean_exit(int dummy) 
{
    int err; 
    
    printf("cleanup\n");    
    if( (err = rt_task_spawn(&clean_ptr, "Clean RT Task", STACK_SIZE, MAX_PRIO, 0, &clean_rt_task, NULL)) < 0){
	    util_pdbg(DBG_CRIT, "Clean: Couldn't launch cleaner\n");
	    exit(err);
    }
    
    while(!end);
    rt_task_delete(&clean_ptr);        
    exit(0);
}

int main( int argc, char** argv )
{
    int err; 
    util_pdbg(DBG_INFO, "MAIN: Starting...\n");		

    // Assure a clean exit
    signal(SIGTERM, clean_exit);	
    signal(SIGINT, clean_exit);	   
    
    // Avoid page faults	
    if( ( err = mlockall(MCL_CURRENT | MCL_FUTURE)) < 0 ) {
	    util_pdbg(DBG_CRIT, "MAIN: Memory could not be locked. Exiting...\n");
	    exit(-1);
    }

    print_banner();	

    if( (err = rt_task_spawn(&main_task_ptr, "Main task", STACK_SIZE, MAX_PRIO, 0, &main_task, NULL)) < 0){
	    util_pdbg(DBG_CRIT, "MAIN: Main task could not be correctly initialized\n");
	    exit(err);
    }

    if( (err = rt_task_spawn(&watchdog_ptr, "Watchdog", STACK_SIZE, WATCHDOG_PRIO, 0, &watchdog, NULL)) < 0){
	    util_pdbg(DBG_CRIT, "MAIN: Watchdog could not be correctly initialized\n");
	    exit(err);
    }
    
    // wait for signal & return of signal handler
    pause();
    fflush(NULL);
    
    return 0; 
}

