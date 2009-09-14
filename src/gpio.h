#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_TRISTATE_OFFSET 1 


// enum{
//     LEFT = 0x01, 
//     RIGHT = 0x02,
//     UP = 0x03,
//     DOWN = 0x04,
//     CENTER = 0x05
// } directions; 

int map_gpio();

int unmap_gpio();

inline void write_led4(uint8_t val);

inline void write_led_dir(uint8_t val);

// Polling
inline uint8_t read_buttons();

#endif
