/** ******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
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

*  ******************************************************************************* **/

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/types.h>
#include <time.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

 #include "version.h"         // LIB VERSION
 #include "gpio.h"            // GPIO 
 #include "platform_io.h"     // High-level functions for GPIO devices

// #include "lcd_proc.h"        // 16x2 LCD 
// #include "openloop_motors.h" // MOTORS
// #include "hwservos.h"	     // SERVOS

#define DEVSPI "/dev/spi0"


void geninputs_isr(void *cookie) {
// 	int err = 0;
// 	int error;
// 	error = rt_pipe_create(&mypipe, "intpipe", PIPE_MINOR, 0);
// 	// todo: add error handling
// 	while (!end) {
// 		err = rt_intr_wait(&intr_desc, TM_INFINITE);
// 		if (err > 0) {
// 			int_count++;
// 			rt_pipe_write(&mypipe, &int_count, sizeof(int_count), P_NORMAL);
// 		} else {
// 			//printf("interrupt_task error = %i\n", err);
// 		}
// 		rt_intr_enable (&intr_desc);
// 	}
// 	rt_pipe_delete(&mypipe);

	// if buttons, then ligth the equivalent led, otherwise ignore.
}

void test_gpio()
{
    int res; 
    /* Init all of the 
    if( ( res = pio_initall(geninputs_isr, NULL) ) < 0 ){
	util_pdbg(DBG_CRIT, "Error initializing GPIOs %d", res);
	exit(res); 
    }
	
}

*/

int main( int argc, char** argv)
{

    int opts; 

    while (1) {

        int option_index = 0;

        static struct option long_options[] = 
        {
            {"gpio",        no_argument,    0,   'g'},
            {"compass",     no_argument,    0,   'c'},
            {"acc",         no_argument,    0,   'a'},
            {"sonar",       no_argument,    0,   's'},
            {"temperature", no_argument,    0,   't'},
            {"adc",         no_argument,    0,   'm'},
            {"motors",      no_argument,    0,   'M'},
	    {"servos",      no_argument,    0,   'S'}, 
            {"help",        no_argument,    0,   'h'},
            {0, 0, 0, 0}
        };

        opts = getopt_long(argc, argv, "acgmsSMth",
                 long_options, &option_index);

        // Detect the end of the options
        if ( opts == -1 ) 
            break;

        switch (opts) {
        case 0:
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case 'g':
            test_gpio();
            break;

        case 'a':
//             test_accelerometer();
            break;

        case 'c':
//             test_compass();
            break;

        case 's':
//             test_sonar();
            break;

        case 'm':
//             test_adc();
            break;

        case 't':
//             test_temperature();
            break;

        case 'M':
//             test_motors();
            break;

        case 'S':
//             test_servos();
            break;

        case 'h':
            printf( " \nUsage:\n 
		      GPIO test\t	 -g\n 
		      Compass\t		 -c\n 
		      Accelerometer\t	 -a\n 
		      Sonar\t 		 -s\n 
		      Temperature\t      -t\n 
		      ADC\t 		 -m\n 
		      Motors\t 		 -M\n 
		      Servos\t 		 -S\n 
		      \n");
            break;

        case '?':
            break;

        default:
            printf("Incorrect option 0x%x!\n", opts);
        }

    }    

    return 0; 
}

// void test_compass()
// {
//     int i,res; 
//     uint16_t deg;
// 
//     /* Testing I2C COMPASS */
//     printf("Testing compass....\n"); 
//     if(hmc6532_idcheck(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS) < 0 )
//         printf("error!\n");
//     else
//         printf("ID successful\n");
// 
//     printf("Initializing in query mode\n") ; 
// 
//      if(( res =  hmc6532_init_query(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS)) < 0 ){
//          printf(" ERROR when initializing compass: %d \n" , res ) ; 
//          exit(1);
//      }
// 
//     printf("Performing 5 measures\n") ; 
//     //TODO: fix - first measure is rubbish (actually correspond with last read -> eeprom address register )
//     hmc6532_read_nowait(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS,&deg ); 
// 
//     for( i = 0 ; i < 5 ; i++ )
//     {
//         sleep(1); // wait for the first measures
//         if(( res = hmc6532_read_nowait(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS,&deg )) < 0 ){
//             printf(" ERROR when reading compass: %d \n" , res ); 
//             exit(1);
//         }
// 
//         printf("0x%x\t Degrees: %f\n", deg, ((float)deg)/10); 
//     }
// 
//     printf("Initializing in standby mode\n") ; 
//     
//      if(( res =  hmc6532_init_standby(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS)) < 0 ){
//          printf(" ERROR when initializing compass: %d \n" , res ) ; 
//          exit(1);
//      }
// 
//     printf("Performing 5 measures\n") ; 
//     //TODO: fix - first measure is rubbish (actually correspond with last read -> eeprom address register )
//     hmc6532_read_wait(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS,&deg ); 
// 
//     for( i = 0 ; i < 5 ; i++ )
//     {
//         sleep(1); // wait for the first measures
//         if(( res = hmc6532_read_wait(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS,&deg )) < 0 ){
//             printf(" ERROR when reading compass: %d \n" , res ); 
//             exit(1);
//         }
// 
//         printf("0x%x\t Degrees: %f\n", deg, ((float)deg)/10); 
//     }
// 
//     //----------- Continous -----------------------------------
//     printf("Initializing in continous mode 10Hz\n") ; 
// 
//      if(( res =  hmc6532_init_continous(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS ,10)) < 0 ){
//          printf(" ERROR when initializing compass: %d \n" , res ) ; 
//          exit(1);
//      }
//     //TODO: fix - first measure is rubbish (actually correspond with last read -> eeprom address register )
//     hmc6532_read_nowait(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS,&deg ); 
// 
//     printf("Performing 5 measures\n") ; 
//     for( i = 0 ; i < 5 ; i++ )
//     {
//         sleep(1); // wait for the first measures
//         if(( res = hmc6532_read_nowait(I2C_MAGNETICCOMPASS_ADDRESS,I2C_MAGNETICCOMPASS_BUS,&deg )) < 0 ){
//             printf(" ERROR when reading compass: %d \n" , res ); 
//             exit(1);
//         }
// 
//         printf("0x%x\t Degrees: %f\n", deg, ((float)deg)/10); 
//     }
// 
// }
// 
// void test_accelerometer()
// {
//     lis3s lis3data;
//     double x,y,z;
//     /* Testing I2C/ACCELEROMETER */
// 
//     lis3lv02dl_init_3axis(I2C_ACCELEROMETER_ADDRESS, I2C_ACCELEROMETER_BUS);
//     printf("Move the thing!\n");
// 
//     if( lis3lv02dl_calib(I2C_ACCELEROMETER_ADDRESS, I2C_ACCELEROMETER_BUS, &lis3data) < 0 ){
//         printf("Cannot read!");
//         exit(1);
//     }else{
//         printf("Calibration X = %d\tY = %d\tZ = %d\n", lis3data.xcal,lis3data.ycal,lis3data.zcal); 
//     }
// 
//     while(1)
//     {
//         sleep(1);
//         if( lis3lv02dl_read(I2C_ACCELEROMETER_ADDRESS, I2C_ACCELEROMETER_BUS, &lis3data) < 0 ){
//             printf("Cannot read!");
//         }
// 
//         x = (double)(lis3data.xacc - lis3data.xcal)/(SCALE_FACTOR_6G_16bit); 
//         y = (double)(lis3data.yacc - lis3data.ycal)/(SCALE_FACTOR_6G_16bit); 
//         z = (double)(lis3data.zacc - lis3data.zcal)/(SCALE_FACTOR_6G_16bit); 
// 
//         printf("X = %f\tY = %f\tZ = %f\n", x,y,z);
// 
//     }
// 
// }
// void test_temperature()
// {
//     int16_t temp;
// 
//     /* Testing I2C/TEMP SENSOR */
// 
//     printf("Testing temp sensor\n");
//     tcn75_init(I2C_TEMPSENSOR_ADDRESS, I2C_TEMPSENSOR_BUS );
// 
//     while(1)
//     {
//         tcn75_read(I2C_TEMPSENSOR_ADDRESS, I2C_TEMPSENSOR_BUS, &temp);
//         printf(" Temp read: %.1f ºC\n", (double)temp/2 ); 
//         sleep(1);
//     }
// }
// 
// void test_sonar()
// {
//      int i;
//     /* Testing I2C/SONAR */
//     printf("Firing SONAR 0\n");
// 
// //     i2cset(I2C_SONAR0_ADDRESS,0x00, 0x51,I2C_SONAR0_BUS, 'b' );
// //     usleep(60000);
// // 
// //     for( i = 0 ; i < 6 ; i++){
// //         res = i2cget(I2C_SONAR0_ADDRESS,i,I2C_SONAR0_BUS, 'b');
// //         printf("Reg[0x%x]=%d\n",i,res);
// //     }
// 
//     srf08_fire_cm(I2C_SONAR0_ADDRESS, I2C_SONAR0_BUS ); 
//     srf08_sleep_max();
//     printf("Light: 0x%x\n", srf08_get_light( I2C_SONAR0_ADDRESS, I2C_SONAR0_BUS ));
// 
//     for ( i = 0 ; i < 17 ; i++ ) 
//         printf( "Echo[%d]: %d (cm)\n", i , srf08_get_echo(I2C_SONAR0_ADDRESS, I2C_SONAR0_BUS, i ));
// 
// }
// 
// void test_adc()
// {
//     xspidev xspi; 
//     int i; 
//     uint8_t* dest; 
//     max1231_config adcconf; 
//     /* Testing SPI / ADC */    
//     spi_configure(&xspi, DEVSPI, 0, 0 , 0 , 0 , 0 , 0 , 0 , 0, 0); // TODO: Explain here
//     spi_set_config(&xspi);
// 
//     // Configuration with Gyros in differential mode
//     /*
//     adcconf.pairs[4] = MAX1231_CONF_BIPDIFF_MASK ;  // Gyro in differential mode
//     adcconf.pairs[5] = MAX1231_CONF_BIPDIFF_MASK ;  // Gyro in differential mode
//     adcconf.pairs[6] = MAX1231_CONF_BIPDIFF_MASK ;  // Gyro in differential mode
//     adcconf.clock = 0x64 ; 
//     adc_config(&xspi, &adcconf );
//     */
//   
//     // Configuration with Gyros in unipolar mode
//     adc_reset(&xspi); 
//     usleep(10000); // needed? 
//     adc_config_all_uni_single(&xspi);
//     usleep(10000); // needed?
// 
//     printf("Reading inputs in no-scan mode\n");
//     printf("Temperature %f ºC \n",  adc_get_temperature(&xspi));
//     adc_reset_fifo(&xspi);
// 
//     for( i = 0 ; i < 16 ; i++ )
//     {
//         printf("ADC_CHANNEL %d:\n", i);
//         printf("%d\n",adc_read_one_once(&xspi,i));// scan 1 channel
//     }
// 
// 
//     dest = (uint8_t*) malloc(32) ; // 2*sizeof(uint8_t)
// 
//     printf("Reading inputs in scan mode\n");
// 
//     if( adc_read_scan_0_N(&xspi,dest, 15) < 0 )
//         printf("Problems reading in Scan Mode 0 to N\n");
//     else{
//             for( i = 0 ; i < 16 ; i++ )
//             {
//                 printf("ADC CHANNEL %d:", i);
//                 printf("%d\n",(int)( (int)dest[i*2+1] | ((int)dest[i*2]<<8)) );// scan 1 channel
//             }
//     }
// 
//     free(dest); 
// 
// }

// void test_motors()
// {
//     int i,j; 
//     volatile int* p; 
//     int k, l; 
//     l = 0; 
//     
//     printf("Mapping Motor and Quadrature encoders\n");
//     map_motors();
//     map_qenc();    
//     map_servos();
//     
//     for(i = 0 ; i < 4 ; i++)     
//       oloopm_set_freq_div(i,3);
//     
//     for(i = 0 ; i < 4 ; i++)     
//       oloopm_set_speed(i,0);
//     sleep(1);
//     
//     printf("Moving forward at +1000 for 1 second\n");
//     for(i = 0 ; i < 4 ; i++)     
// 	oloopm_set_speed(i,800);        
//     sleep(1);
//     
//     printf("Moving backward at -300 for 1 second\n");
//     for(i = 0 ; i < 4 ; i++)     
// 	oloopm_set_speed(i,-800);
//     sleep(1);
// 
//     for(i = 0 ; i < 4 ; i++)
//       {
// 	printf("pulse_count(%d):%d\n",i, oloopenc_read_pulsecount(i) );    
//       }
//     printf("Clearing counters...\n");
//     for(i = 0 ; i < 4 ; i++)     
// 	oloopenc_setzero(i);
// 
//     printf("Moving forward for 30 seconds and counting\n");
//     for(i = 0 ; i < 4 ; i++)     
// 	oloopm_set_speed(i,800);        
//     sleep(1);
//  
//     k = 1000;
//      
//     while(1){
//  	    l++;
// 	    for( j = 0 ; j < 6 ; j++ ) 
// 	    { 
// 	      k = -k ; 
//               timestamp();
// 	      for(i = 0 ; i < 4 ; i++)
// 	      {
// 		printf("%d - pulse_count(%d):%d\n",l,i, oloopenc_read_pulsecount(i) );    
// 		oloopm_set_speed(i,k);
// 	        servo_set_pos(i, HWSERVOS_TIME_MAX_ANGLE );       
// 	      }
// 	      sleep(1); 
// 	      k = -k ; 
//               timestamp();
// 	      for(i = 0 ; i < 4 ; i++)
// 	      {
// 		printf("%d - pulse_count(%d):%d\n",l,i, oloopenc_read_pulsecount(i) );    
// 		oloopm_set_speed(i,k);
// 	        servo_set_pos(i, HWSERVOS_TIME_MIN_ANGLE );       
// 	      }
// 	      sleep(1); 
// 	    }
//             printf("Setting encoders to 0 \n");
// 
//             for(i = 0 ; i < 4 ; i++)
// 	    {
// 		    oloopenc_setzero(i); 
// 	    }
//     }
//     printf("Stopping\n");
//     for(i = 0 ; i < 4 ; i++)     
//       oloopm_set_speed(i,0);
// 
//     unmap_motors();
//     unmap_qenc();
// }
// 
// void test_servos()
// {
//   int i,j; 
// 
//   printf("Mapping Servos\n");
//   map_servos();
// 
//   printf("Move to minimum angle %d us\n",HWSERVOS_TIME_MIN_ANGLE );  
//   for(i = 0 ; i < HWSERVOS_NUM_OF ; i++) 
//     servo_set_pos(i, HWSERVOS_TIME_MIN_ANGLE );
//   sleep(2);
//   
//   printf("Move to maximum angle %d us\n", HWSERVOS_TIME_MAX_ANGLE );
//   for(i = 0 ; i < HWSERVOS_NUM_OF ; i++) 
//     servo_set_pos(i, HWSERVOS_TIME_MAX_ANGLE );
//   sleep(2);
//   
//   printf("Move to the center %d us \n", HWSERVOS_TIME_MID_ANGLE);
//   for(i = 0 ; i < HWSERVOS_NUM_OF ; i++) 
//     servo_set_pos(i, HWSERVOS_TIME_MID_ANGLE );
//   sleep(2);
//   
//   printf("Moving through the whole range\n");
//   for( i = HWSERVOS_TIME_MIN_ANGLE; i < HWSERVOS_TIME_MAX_ANGLE  ; i += 10 ){
//     for(j = 0 ; j < HWSERVOS_NUM_OF ; j++) 
//       servo_set_pos(j , i );
//     
//     usleep(24000);
//   }
// 
//   printf("Move to the center %d us \n", HWSERVOS_TIME_MID_ANGLE);
//   for(i = 0 ; i < HWSERVOS_NUM_OF ; i++) 
//     servo_set_pos(i, HWSERVOS_TIME_MID_ANGLE );
//   
//   sleep(2);
//   
//   printf("Unmapping Servos\n");
//   unmap_servos();
// }
