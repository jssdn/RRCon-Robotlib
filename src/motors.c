/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: openloop_motors.c Functions for the basic DC motor control with quadrature enconder
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
//Xenomai
#include <native/mutex.h>

#include "busio.h"
#include "motors.h"
#include "util.h"
#include "dev_mmaps_parms.h"

/* Motors */
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
	    util_pdbg(DBG_WARN, "MOTORS: Couldn't map PWM: %d\n", err);
	    goto pwm_nothing;
	}
	motor->madd_base = madd_base; 
	motor->madd_end = madd_end; 
	/* Allocate memory for each core */
	if( (motor->pwms= malloc(sizeof(PWM)*num_of_motors) ) == NULL){	
	    util_pdbg(DBG_WARN, "MOTORS: Cannot allocate memory" );
	    goto pwm_unmap;
	}	
	motor->num_of_motors = num_of_motors; 
	/* Create RT Mutexes */
	for( im = 0 ; im < num_of_motors ; im++ ){
	    if( (err = rt_mutex_create(&(motor->pwms[im]->mutex), 0)) < 0 ){
		    util_pdbg(DBG_WARN, "MOTORS: Error rt_mutex_create: %d\n", err);
		    goto pwm_clean_mutex;
	    }
	}
	/* Set default parameters */
	//TODO:
    }

    /* Quadrature encoders instances */
    if( num_of_encs > 0 && num_of_encs <= QENC_MAX_NUM_OF_CORES){
	util_pdbg(DBG_INFO, "MOTORS: Mapping quadrature encoders... \n");

	/* Map motors */
	if( (err = mapio_region(&(motor->qadd), qadd_base, qadd_end)) < 0 ){
	    util_pdbg(DBG_WARN, "MOTORS: Couldn't map QENC: %d\n", err);
	    goto qenc_nothing;
	}
	motor->qadd_base = qadd_base; 
	motor->qadd_end = qadd_end; 
	/* Allocate memory for each core */
	if( (motor->encoders= malloc(sizeof(PWM)*num_of_motors) ) == NULL){	
	    util_pdbg(DBG_WARN, "MOTORS: Cannot allocate memory" );
	    goto qenc_unmap;
	}	
	/* Create RT Mutexes */
	for( iq = 0 ; iq < num_of_motors ; iq++ ){
	    if( (err = rt_mutex_create(&(motor->pwms[im]->mutex), "PWM Reg Mutex")) < 0 ){
		    util_pdbg(DBG_WARN, "MOTORS: Error rt_mutex_create: %d\n", err);
		    goto qenc_clean_mutex;
	    }
	} 
	/* Set default parameters */
	//TODO:
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
	    if( rt_mutex_delete(&(motor->encoders[iq]->mutex)) < 0 )
		    util_pdbg(DBG_WARN, "MOTORS: Error cleaning mutexes: %d\n", err);
    free(motor->encoders);
qenc_unmap: 
    if ( (err = unmapio_region(&(motor->qadd), qadd_base, qadd_end)) < 0 ){
	util_pdbg(DBG_WARN, "MOTORS: QENC couldn't be unmapped at virtual= %ld . Error : %d \n", &(motor->qadd), err);
	return err; 
    }
qenc_nothing:
pwm_clean_mutex: 
    if( im != 0 ) 
	for( im -= 1 ;im >= 0; im-- ) 
	    if( rt_mutex_delete(&(motor->pwms[im]->mutex)) < 0 )
		    util_pdbg(DBG_WARN, "MOTORS: Error cleaning mutexes: %d\n", err);
    free(motor->pwms);
pwm_unmap: 
    if ( (err = unmapio_region(&(motor->madd), madd_base, madd_end)) < 0 ){
	util_pdbg(DBG_WARN, "MOTORS: PWM couldn't be unmapped at virtual= %ld . Error : %d \n", &(motor->madd), err);
	return err; 
    }
pwm_nothing: 
    return err; 

}

int motors_clean_motor(MOTOR* motor)
{
    int err;
    int i; 

    util_pdbg(DBG_INFO, "MOTORS: Cleaning motor driver\n");
    util_pdbg(DBG_DEBG, "MOTORS: Cleaning encoders\n");

    for( i = 0 ;i <= motor->num_of_encs; i-- ) 
	if( (err = rt_mutex_delete(&(motor->encoders[i]->mutex))) < 0 ){
		util_pdbg(DBG_WARN, "MOTORS: Error cleaning mutexes: %d\n", err);
		return err; 
	}
    free(motor->encoders);

    if ( (err = unmapio_region(&(motor->qadd), motor->qadd_base, motor->qadd_end)) < 0 ){
	util_pdbg(DBG_WARN, "MOTORS: PWM couldn't be unmapped at virtual= %ld . Error : %d \n", &(motor->qadd), err);
	return err; 
    }

    util_pdbg(DBG_DEBG, "MOTORS: Cleaning pwms\n");

    for( i = 0 ;i <= motor->num_of_motors; i-- ) 
	if( (err = rt_mutex_delete(&(motor->pwms[i]->mutex))) < 0 ){
		util_pdbg(DBG_WARN, "MOTORS: Error cleaning mutexes: %d\n", err);
		return err; 
	}
    free(motor->pwms);

    if ( (err = unmapio_region(&(motor->madd), motor->madd_base, motor->madd_end)) < 0 ){
	util_pdbg(DBG_WARN, "MOTORS: PWM couldn't be unmapped at virtual= %ld . Error : %d \n", &(motor->madd), err);
	return err; 
    }

    return 0; 
}


/* PWM */

// Set carrier divider 
int motors_pwm_set_freq_div(MOTOR* motor, unsigned motnum, int value)
{
    int err; 

    value = (value > MOTORS_MAX_FREQ_DIV) ? MOTORS_MAX_FREQ_DIV : value ;
    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    if( (err = rt_mutex_acquire(&(motor->pwms[motnum]->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;

    *(motor->madd + (motnum<<1) + 1) = value ; 
    motor->pwms[motnum]->freq_div = value; 

    if( (err = rt_mutex_release(&(motor->pwms[motnum]->mutex))) < 0 )
	return err; 
    
    return 0; 
}
// Read value from carrier divider
int motors_pwm_read_freq_div(MOTOR* motor, unsigned motnum, unsigned* ret)
{
    int err; 

    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    if( (err = rt_mutex_acquire(&(motor->pwms[motnum]->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;

    *ret =  motor->pwms[motnum]->freq_div; 

    if( (err = rt_mutex_release(&(motor->pwms[motnum]->mutex))) < 0 )
	return err; 

    return 0; 
}
// Set speed in DUTY CYCLE = speed/1023
int motors_pwm_set_speed(MOTOR* motor, int motnum, int value)
{
    int err; 

    value = (value > MOTORS_MAX_SPEED) ? MOTORS_MAX_SPEED : value ;
    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    if( (err = rt_mutex_acquire(&(motor->pwms[motnum]->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;

    *(motor->madd + (motnum<<1)) = value ; 
    motor->pwms[motnum]->speed = value; 

    if( (err = rt_mutex_release(&(motor->pwms[motnum]->mutex))) < 0 )
	return err; 
    
    return 0; 
}
// Read speed in DUTY CYCLE = speed/1023
int motors_pwm_read_speed(MOTOR* motor, unsigned motnum, int* ret)
{
    int err; 

    motnum = ( motnum > motor->num_of_motors ) ? motor->num_of_motors : motnum;

    if( (err = rt_mutex_acquire(&(motor->pwms[motnum]->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;
    
    *ret =  motor->pwms[motnum]->speed; 

    if( (err = rt_mutex_release(&(motor->pwms[motnum]->mutex))) < 0 )
	return err; 

    return 0; 
}

/* Encoders */

// Reset to zero the encoder values and log registers 
int motors_qenc_setzero(MOTOR* motor, unsigned qencnum )
{
    int err; 

    qencnum = ( qencnum > motor->num_of_encs ) ? motor->num_of_encs : qencnum;

    if( (err = rt_mutex_acquire(&(motor->encoders[qencnum]->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;
    // A glitch here will reset the counter
    *(motor->qadd + qencnum) = 1; 
    *(motor->qadd + qencnum) = 0; 

    motor->encoders[qencnum]->qenc_value = 0; 
    motor->encoders[qencnum]->qenc_prev_value = 0; 

    if( (err = rt_mutex_release(&(motor->encoders[qencnum]->mutex))) < 0 )
	return err; 

    return 0; 
}
// Read the number of pulses read during the movement by the quadrature decoder (signed integer)
int motors_qenc_read_enc(MOTOR* motor, unsigned qencnum, int* value, int* prev_value )
{
    int err;

    qencnum = ( qencnum > motor->num_of_encs ) ? motor->num_of_encs : qencnum;

    if( (err = rt_mutex_acquire(&(motor->encoders[qencnum]->mutex), TM_INFINITE)) < 0)  // block until mutex is released
	return err;

    motor->encoders[qencnum]->qenc_prev_value = motor->encoders[qencnum]->qenc_value;
    motor->encoders[qencnum]->qenc_value = *(motor->qadd + qencnum); 
    
    *value = motor->encoders[qencnum]->qenc_value;
    *prev_value = motor->encoders[qencnum]->qenc_prev_value;

    if( (err = rt_mutex_release(&(motor->encoders[qencnum]->mutex))) < 0 )
	return err; 

    return 0; 
}

/* PID */

//TODO: To be implemented
// Set params
void motors_pid_set_params(MOTOR* motor, unsigned pidnum, int kp, int ki, int kd)
{
}
// Read params
int motors_pwm_read_kp(MOTOR* motor, unsigned pidnum, int* ret)
{   
    return 0; 
}

int motors_pwm_read_ki(MOTOR* motor, unsigned pidnum, int* ret)
{    
    return 0;
}

int motors_pwm_read_kd(MOTOR* motor, unsigned pidnum, int* ret)
{
    return 0;
}
