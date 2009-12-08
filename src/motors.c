/**
    @file motors.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief Functions for the basic DC motor control with quadrature enconder
    
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
    
    @note PID functions not implemented yet

    @version 0.4-Xenomai       
    
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/types.h>
//Xenomai
#include <native/mutex.h>
//--
#include "busio.h"
#include "motors.h"
#include "util.h"
#include "dev_mmaps_parms.h"

/**
* @brief Init MOTORS device structure 
*
* @param motor Motor structure to use. Acts as an unified structure for Motors and Encoders.
* @param madd_base Base address for the memory mapped PWM peripheral
* @param madd_end End address for the memory mapped PWM peripheral
* @param num_of_motors Number of motors attached to the PWM peripheral
* @param qadd_base Base address for the memory mapped Quadrature Encoder peripheral
* @param qadd_end End address for the memory mapped Quadrature Encoder peripheral
* @param num_of_motors Number of encoders attached to the Quadrature Encoder peripheral
* @param padd_base Base address for the memory mapped PID peripheral( Not implemented yet... )
* @param padd_end End address for the memory mapped PID peripheral( Not implemented yet... )
* @param num_of_motors Number of PIDs attached to the PID peripheral( Not implemented yet... )
* @return 0 on success. Otherwise error. 
*
* Initializes all the peripherals unifying their control under the MOTOR device structure. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int motors_init_motor(MOTOR* motor, 
		      unsigned long madd_base, unsigned long madd_end, unsigned num_of_motors, 
		      unsigned long qadd_base, unsigned long qadd_end, unsigned num_of_encs, 
		      unsigned long padd_base, unsigned long padd_end, unsigned num_of_pids)
{
    int im= 0;
    int iq= 0; 
//     int ip= 0; 
    int err; 

    /* PWM instances */
    if( num_of_motors > 0 && num_of_motors <= MOTORS_MAX_NUM_OF_CORES){
	util_pdbg(DBG_INFO, "MOTORS: Mapping motors... \n");

	/* Map motors */
	if( (err = mapio_region(&(motor->madd), madd_base, madd_end)) < 0 ){
	    util_pdbg(DBG_WARN, "\t-> MOTORS: Couldn't map PWM: %d\n", err);
	    goto pwm_nothing;
	}
	motor->madd_base = madd_base; 
	motor->madd_end = madd_end; 
	/* Allocate memory for each core */
	if( (motor->pwms= malloc(sizeof(PWM)*num_of_motors) ) == NULL){	
	    util_pdbg(DBG_WARN, "\t-> MOTORS: Cannot allocate memory" );
	    goto pwm_unmap;
	}	
	motor->num_of_motors = num_of_motors; 
	/* Create RT Mutexes */
	for( im = 0 ; im < num_of_motors ; im++ ){
	    if( (err = rt_mutex_create(&(motor->pwms[im].mutex), 0)) < 0 ){
		    util_pdbg(DBG_WARN, "\t-> MOTORS: Error rt_mutex_create: %d\n", err);
		    goto pwm_clean_mutex;
	    }
	}
	/* Set default parameters */
	//TODO:
    }

    /* Quadrature encoders instances */
    if( num_of_encs > 0 && num_of_encs <= QENC_MAX_NUM_OF_CORES){
	util_pdbg(DBG_INFO, "\t-> MOTORS: Mapping quadrature encoders... \n");

	/* Map encoders */
	if( (err = mapio_region(&(motor->qadd), qadd_base, qadd_end)) < 0 ){
	    util_pdbg(DBG_WARN, "\t-> MOTORS: Couldn't map QENC: %d\n", err);
	    goto qenc_nothing;
	}

	motor->qadd_base = qadd_base; 
	motor->qadd_end = qadd_end; 

	/* Allocate memory for each core */
	if( (motor->encoders= malloc(sizeof(QENC)*num_of_encs) ) == NULL){	
	    util_pdbg(DBG_WARN, "\t-> MOTORS: Cannot allocate memory" );
	    goto qenc_unmap;
	}
	motor->num_of_encs = num_of_encs; 
	/* Create RT Mutexes */
	for( iq = 0 ; iq < num_of_encs ; iq++ ){
 	    if( (err = rt_mutex_create(&(motor->encoders[iq].mutex), 0)) < 0 ){
		    util_pdbg(DBG_WARN, "\t-> MOTORS: Error rt_mutex_create: %d\n", err);
		    goto qenc_clean_mutex;
	    }
	} 

	util_pdbg(DBG_DEBG, "\t-> MOTORS: Setting default parameters\n");

	/* Set default parameters */
	for( im = 0 ;im < motor->num_of_motors; im++ ) 
	    motors_pwm_set_speed(motor, im, 0);	

	/* Reset encoder counters */
	for ( iq = 0 ; iq < QENC_NUM_OF ; iq++)
	    motors_qenc_setzero(motor, iq);

    }

   //TODO:No PID instances implemented yet

//     /* PID Instances */
//     if( num_of_pids > 0 && num_of_pids <= PIDS_MAX_NUM_OF_CORES){
// 	//TODO:No PID instances implemented yet
//     }

    return 0; 

// pid_clean_mutex: 
// pid_deallocate: 
// pid_nothing:


qenc_clean_mutex: 
    if( iq != 0 ) 
	for( iq -= 1 ;iq >= 0; iq-- ) 
	    UTIL_MUTEX_DELETE("MOTOR",&(motor->encoders[iq].mutex));

    free(motor->encoders);
qenc_unmap: 
    if ( unmapio_region(&(motor->qadd), qadd_base, qadd_end) < 0 )
	util_pdbg(DBG_WARN, "MOTORS: QENC couldn't be unmapped at virtual= %ld\n", &(motor->qadd));
qenc_nothing:
pwm_clean_mutex: 
    if( im != 0 ) 
	for( im -= 1 ;im >= 0; im-- ) 
	    UTIL_MUTEX_DELETE("MOTOR",&(motor->pwms[im].mutex));

    free(motor->pwms);
pwm_unmap: 
    if ( unmapio_region(&(motor->madd), madd_base, madd_end) < 0 )
	util_pdbg(DBG_WARN, "MOTORS: PWM couldn't be unmapped at virtual= %ld\n", &(motor->madd));
pwm_nothing: 
    return err; 

}

/**
* @brief Init MOTORS device structure 
*
* @param motor Cleans the MOTOR device structure. 
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int motors_clean_motor(MOTOR* motor)
{
    int err;
    unsigned i; 

    util_pdbg(DBG_INFO, "MOTORS: Cleaning motor driver\n");

    /* Set Speed in each motor */
    for( i = 0 ;i < motor->num_of_motors; i++ )
    {
	util_pdbg(DBG_DEBG, "\t-> MOTORS: Stopping Motor %d\n",i);
	motors_pwm_set_speed(motor, i, 0);	
    }

    util_pdbg(DBG_DEBG, "\t-> MOTORS: Cleaning encoders\n");

    for( i = 0 ;i < motor->num_of_encs; i++ ) 
	UTIL_MUTEX_DELETE("\t-> MOTOR",&(motor->encoders[i].mutex));

    free(motor->encoders);

    if ( (err = unmapio_region(&(motor->qadd), motor->qadd_base, motor->qadd_end)) < 0 ){
	util_pdbg(DBG_WARN, "-> MOTORS: PWM couldn't be unmapped at virtual= %ld . Error : %d \n", &(motor->qadd), err);
	return err; 
    }

    util_pdbg(DBG_DEBG, "\t-> MOTORS: Cleaning pwms\n");

    for( i = 0 ;i < motor->num_of_motors; i++ ) 
	UTIL_MUTEX_DELETE("\t-> MOTOR",&(motor->pwms[i].mutex));

    free(motor->pwms);

    if ( (err = unmapio_region(&(motor->madd), motor->madd_base, motor->madd_end)) < 0 ){
	util_pdbg(DBG_WARN, "-> MOTORS: PWM couldn't be unmapped at virtual= %ld . Error : %d \n", &(motor->madd), err);
	return err; 
    }

    return 0; 
}


/**
* @brief Set the carrier frequency of the PWM through a divider
*
* @param motor MOTOR device structure
* @param motnum Number of the motor attached to the PWM Core
* @param value Value of the frequency divider factor. See note!
* @return 0 on success. Otherwise error. 
*
* That function tells the core which carrier frequency should use for the PWM signal. 
* The recommended "rule of thumb" is between 10Khz and 24 Khz
*
* @note 0 - 190khz 1 - 48 khz 2 - 24 khz 3 - 16 Khz 
* 	4 - 12 Khz 5 - 9.8 Khz 6 - 8 Khz 7 - 7 Khz 
* 	8 - 6 Khz 9 - 5.43 Khz 10 - 4.9 Khz 11 - 4.4 Khz 12 - 4 Khz
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int motors_pwm_set_freq_div(MOTOR* motor, unsigned motnum, int value)
{
    int err; 

    value = (value > MOTORS_MAX_FREQ_DIV) ? MOTORS_MAX_FREQ_DIV : value ;
    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    UTIL_MUTEX_ACQUIRE("MOTORS",&(motor->pwms[motnum].mutex),TM_INFINITE);

    *(motor->madd + (motnum<<1) + 1) = value ; 
    motor->pwms[motnum].freq_div = value; 

    UTIL_MUTEX_RELEASE("MOTORS",&(motor->pwms[motnum].mutex));
    
    return 0; 
}

/**
* @brief Read the carrier frequency of the PWM
*
* @param motor MOTOR device structure
* @param motnum Number of the motor attached to the PWM Core
* @param ret Read value
* @return 0 on success. Otherwise error. 
*
* @note 0 - 190khz 1 - 48 khz 2 - 24 khz 3 - 16 Khz 
* 	4 - 12 Khz 5 - 9.8 Khz 6 - 8 Khz 7 - 7 Khz 
* 	8 - 6 Khz 9 - 5.43 Khz 10 - 4.9 Khz 11 - 4.4 Khz 12 - 4 Khz
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int motors_pwm_read_freq_div(MOTOR* motor, unsigned motnum, unsigned* ret)
{
    int err; 

    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    UTIL_MUTEX_ACQUIRE("MOTORS",&(motor->pwms[motnum].mutex),TM_INFINITE);

    *ret =  motor->pwms[motnum].freq_div; 

    UTIL_MUTEX_RELEASE("MOTORS",&(motor->pwms[motnum].mutex));

    return 0; 
}


/**
* @brief Set the duty cycle on a motor connected to a PWM Core
*
* @param motor MOTOR device structure
* @param motnum Number of the motor attached to the PWM Core
* @param value Value of the duty cycle
* @return 0 on success. Otherwise error. 
*
* Set speed in DUTY CYCLE
*
* @note DUTY CYCLE = speed/1023
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int motors_pwm_set_speed(MOTOR* motor, int motnum, int value)
{
    int err; 

    value = (value > MOTORS_MAX_SPEED) ? MOTORS_MAX_SPEED : value ;
    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    UTIL_MUTEX_ACQUIRE("MOTORS",&(motor->pwms[motnum].mutex),TM_INFINITE);

    *(motor->madd + (motnum<<1)) = value ; 
    motor->pwms[motnum].speed = value; 

    UTIL_MUTEX_RELEASE("MOTORS",&(motor->pwms[motnum].mutex));

    
    return 0; 
}

/**
* @brief Read the duty cycle applied over a motor connected to a PWM Core
*
* @param motor MOTOR device structure
* @param motnum Number of the motor attached to the PWM Core
* @param ret Read value
* @return 0 on success. Otherwise error. 
*
* Set speed in DUTY CYCLE
*
* @note DUTY CYCLE = speed/1023
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int motors_pwm_read_speed(MOTOR* motor, unsigned motnum, int* ret)
{
    int err; 

    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    UTIL_MUTEX_ACQUIRE("MOTORS",&(motor->pwms[motnum].mutex),TM_INFINITE);
    
    *ret =  motor->pwms[motnum].speed; 

    UTIL_MUTEX_RELEASE("MOTORS",&(motor->pwms[motnum].mutex));

    return 0; 
}

/**
* @brief Reset to zero the encoder values and log registers
*
* @param motor MOTOR device structure
* @param qencnum Number of the encoder attached to the QENC Core
* @return 0 on success. Otherwise error. 
*
* This function set the encoder peripheral registers to zero 
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int motors_qenc_setzero(MOTOR* motor, unsigned qencnum )
{
    int err; 

    qencnum = ( qencnum > motor->num_of_encs ) ? motor->num_of_encs : qencnum;

    UTIL_MUTEX_ACQUIRE("MOTORS",&(motor->encoders[qencnum].mutex),TM_INFINITE);

    // A glitch here will reset the counter
    *(motor->qadd + qencnum) = 1; 
    *(motor->qadd + qencnum) = 0; 

    motor->encoders[qencnum].qenc_value = 0; 
    motor->encoders[qencnum].qenc_prev_value = 0; 

    UTIL_MUTEX_RELEASE("MOTORS",&(motor->encoders[qencnum].mutex));

    return 0; 
}

/**
* @brief Read the quadrature decoder readings from the motors.
*
* @param motor MOTOR device structure
* @param qencnum Number of the encoder attached to the QENC Core
* @param value Value of the duty cycle
* @param prev_value Latched value from the previous reading ( timestamping should be performed outside )
* @return 0 on success. Otherwise error. 
*
* Read the number of pulses read during the movement by the quadrature decoder (signed integer)
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int motors_qenc_read_enc(MOTOR* motor, unsigned qencnum, int* value, int* prev_value )
{
    int err;

    qencnum = ( qencnum > motor->num_of_encs ) ? motor->num_of_encs : qencnum;

    UTIL_MUTEX_ACQUIRE("MOTORS",&(motor->encoders[qencnum].mutex),TM_INFINITE);

    motor->encoders[qencnum].qenc_prev_value = motor->encoders[qencnum].qenc_value;
    motor->encoders[qencnum].qenc_value = *(motor->qadd + qencnum); 
    
    *value = motor->encoders[qencnum].qenc_value;
    *prev_value = motor->encoders[qencnum].qenc_prev_value;

    UTIL_MUTEX_RELEASE("MOTORS",&(motor->encoders[qencnum].mutex));

    return 0; 
}

//TODO: To be implemented

/**
* @brief Dummy skeleton for the PID functions.
*
* @note To be implemented
*
*/

void motors_pid_set_params(MOTOR* motor, unsigned pidnum, int kp, int ki, int kd)
{
}

/**
* @brief Dummy skeleton for the PID functions.
*
* @note To be implemented
*
*/
int motors_pwm_read_kp(MOTOR* motor, unsigned pidnum, int* ret)
{   
    return 0; 
}

/**
* @brief Dummy skeleton for the PID functions.
*
* @note To be implemented
*
*/

int motors_pwm_read_ki(MOTOR* motor, unsigned pidnum, int* ret)
{    
    return 0;
}

/**
* @brief Dummy skeleton for the PID functions.
*
* @note To be implemented
*
*/

int motors_pwm_read_kd(MOTOR* motor, unsigned pidnum, int* ret)
{
    return 0;
}
