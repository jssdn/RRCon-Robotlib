#ifndef __OPENLOOP_MOTORS__H__
#define __OPENLOOP_MOTORS__H__

#define MOTORS_BASE 0x75000000
#define MOTORS_END  0x7500ffff

#define QENC_BASE 0x41c00000
#define QENC_END  0x41c0ffff

#define PID_BASE 0x75020000
#define PID_END  0x7502ffff

#define OLOOP_MAX_SPEED 1023 /* 11 bits signed */
#define OLOOPM_MAX_FREQ_DIV 255 /* 8 bits unsigned */

enum motors{ MOTOR1 = 0,
             MOTOR2 = 1,
             MOTOR3 = 2,
             MOTOR4 = 3 };

//Motors
int map_motors();

int unmap_motors();

// Set carrier divider 
inline void oloopm_set_freq_div( enum motors m, unsigned n);
// Read value from carrier divider
inline unsigned int oloopm_read_freq_div(enum motors m);
// Set speed in DUTY CYCLE = speed/1024
inline void oloopm_set_speed( enum motors m, int speed);
// Read speed in DUTY CYCLE = speed/1024
inline unsigned int oloopm_read_speed(enum motors m);

//Encoders
int map_qenc();

int unmap_qenc();
// Reset to zero the encoder values 
inline void oloopenc_setzero(enum motors m); 
// Read the number of pulses read during the movement by the quadrature decoder (signed integer)
inline signed int oloopenc_read_pulsecount(enum motors m);

#endif
