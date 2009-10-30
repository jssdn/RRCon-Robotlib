#ifndef __OPENLOOP_MOTORS__H__
#define __OPENLOOP_MOTORS__H__

typedef struct{
    unsigned freq_div; 
    unsigned speed; 
    RT_MUTEX mutex;
} PWM; 

typedef struct{   
    int qenc_value; // Value in T
    int qenc_prev_value; // Value in T+1 
    RT_MUTEX mutex;
} QENC; 

typedef struct{    
    int kp;
    int ki; 
    int kd; 
    RT_MUTEX mutex;
} PID; 

typedef struct{
    volatile int* madd; 
    unsigned long madd_base; 
    unsigned long madd_end; 
    unsigned num_of_motors;
    volatile int* qadd;
    unsigned long qadd_base; 
    unsigned long qadd_end; 
    unsigned num_of_encs;
    volatile int* padd;      
    unsigned long padd_base; 
    unsigned long padd_end; 
    unsigned num_of_pids;
    // Dynamically allocated array for the arbitrary number of motors that the core can handle 
    PWM* pwms; // array of structs
    QENC* encoders; // array of structs
    PID* pids; // array of structs
} MOTOR; 


/* Motors */
int motors_init_motor(MOTOR* motor, 
		      unsigned long madd_base, unsigned long madd_end, unsigned num_of_motors, 
		      unsigned long qadd_base, unsigned long qadd_end, unsigned num_of_encs, 
		      unsigned long padd_base, unsigned long padd_end, unsigned num_of_pids);

int motors_clean_motor(MOTOR* motor);

/* PWM */

// Set carrier divider 
int motors_pwm_set_freq_div(MOTOR* motor, unsigned motnum, int value);

// Read value from carrier divider
int motors_pwm_read_freq_div(MOTOR* motor, unsigned motnum, unsigned* ret);

// Set speed in DUTY CYCLE = speed/1023
int motors_pwm_set_speed(MOTOR* motor, int motnum, int value);

// Read speed in DUTY CYCLE = speed/1023
int motors_pwm_read_speed(MOTOR* motor, unsigned motnum, int* ret);

/* Encoders */

// Reset to zero the encoder values and log registers 
int motors_qenc_setzero(MOTOR* motor, unsigned qencnum );

// Read the number of pulses read during the movement by the quadrature decoder (signed integer)
int motors_qenc_read_enc(MOTOR* motor, unsigned qencnum, int* value, int* prev_value );


/* PID */

//TODO: To be implemented
// Set params
void motors_pid_set_params(MOTOR* motor, unsigned pidnum, int kp, int ki, int kd);

// Read params
int motors_pwm_read_kp(MOTOR* motor, unsigned pidnum, int* ret);

int motors_pwm_read_ki(MOTOR* motor, unsigned pidnum, int* ret);

int motors_pwm_read_kd(MOTOR* motor, unsigned pidnum, int* ret);
 
#endif
