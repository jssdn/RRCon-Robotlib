/**
    @file lcd.h
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief [HEADER] High Level 16x2 LCD functions for a 4bit interface
*/

#ifndef __LCD_H__
#define __LCD_H__

#include "gpio.h"
#include <native/mutex.h>

//TODO: Out from here!
#define LCD_BASE  0x81480000
#define LCD_END   0x8148FFFF


// LCD commands

#define LCD_SET  (0x28)		///< b000101000 function set, 4 bit, 2 line, 5x7
#define LCD_SET8 (0x38)		///< b000111000 function set, 8 bit, 2 line, 5x7
#define LCD_ON   (0x0c)		///< b000001100 display on, cursor off, blink off
#define LCD_OFF  (0x08)		///< b000001000 display off, cursor off, blink off
#define LCD_MD   (0x06)		///< b000000110 entry mode, increment, no shift
#define LCD_ADR  (0x80)		///< b010000000 set address to zero
#define LCD_ADR2 (0xA8)		///< b010000000 set address to zero
#define LCD_CLR  (0x01)		///< b000000001 clear display

typedef struct{
    //TODO: or this should be a pointer and the allocation goes somewhere else?
    GPIO gpio; ///< GPIO of the LCD that is being used 
    RT_MUTEX mutex; 
} LCD;

/*
 Low-level functions
 */ 

/* 
 *   lcd_send_data_4bit : Send data in two times in 4 bit mode
 */ 

int lcd_send_data_4bit(LCD* lcd, char rs, char data);

/* 
 *   lcd_send_nibble: Send only a nibble( for special commands )
 */ 

int lcd_send_nibble(LCD* lcd, char rs, char data);

/* 
 *  High level simplifications for using the LCD
 */

inline int lcd_on(LCD* lcd);

inline int lcd_off(LCD* lcd);

inline int lcd_clear(LCD* lcd);

/* 
 *   lcd_init: Inits the lcd in 4bit mode
 */ 

int lcd_init(LCD* lcd);
int lcd_clean(LCD* lcd);


int lcd_print(LCD* lcd, const char *msg);

#endif
