#ifndef __HWSERVOS__H__
#define __HWSERVOS__H__

#define HWSERVOS_BASE 0x73600000
#define HWSERVOS_END  0x7360FFFF

//#define HWSERVOS_BASE 0x41a00000
//#define HWSERVOS_END  0x41a0FFFF

// The following values should match with the defined in hardware
#define HWSERVOS_TIME_MAX_ANGLE 2200 /* 2 ms to reach the maximum aperture (typ 2-2.2ms) */
#define HWSERVOS_TIME_MIN_ANGLE 800 /* 1 ms to reach the minimum aperture (typ 0.8-1ms) */
#define HWSERVOS_TIME_MID_ANGLE 1500 /* 1.5 ms to reach the minimum aperture (typ 0.8-1ms) */

#define HWSERVOS_NUM_OF 8 /* Number of servos managed by the core */

enum servos{ SERVO1 = 0,
             SERVO2 = 1,
             SERVO3 = 2,
             SERVO4 = 3,
             SERVO5 = 4,
             SERVO6 = 5,
             SERVO7 = 6,
             SERVO8 = 7
 };

int map_servos();
int unmap_servos();

void servo_set_pos(enum servos s, unsigned int val);

inline void servo_enable(enum servos s);

inline void servo_disable(enum servos s);

#endif
