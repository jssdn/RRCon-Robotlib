/** *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:  Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 

    Code: lcd.c High Level 16x2 LCD functions for a 4bit interface
	        
    License: Licensed under GPL2.0 

    Copyright (C) Jorge Sánchez de Nova
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
    
    NOTE: Needs testing!
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
#include <errno.h>
//Xenomai
#include <native/mutex.h>
//--
#include "lcd.h"
#include "dev_mmaps_parms.h"
#include "gpio.h"
#include "util.h"

/**
 Low-level functions
 **/ 

/* 
 *    lcd_data_latch : latch data into the lcd by toggling the enable
 */ 

int lcd_data_latch(LCD* lcd, char rs, char data4b)
{
    int err; 
    
    if ( (err = gpio_write(&(lcd->gpio), LCD16X2_LCD_MASK, 0, 0, (rs | data4b))) < 0 )
	return err;     
    __usleep(41);
    if ( (err = gpio_write(&(lcd->gpio), LCD16X2_LCD_MASK, 0, 0, (0x40 | rs | data4b))) < 0 )
	return err; 
    __usleep(41);  
    if ( (err = gpio_write(&(lcd->gpio), LCD16X2_LCD_MASK, 0, 0, (rs | data4b ))) < 0 )
	return err;     
    __usleep(41);
    return 0;
}

/* 
 *   lcd_send_data_4bit : Send data in two times in 4 bit mode
 */ 

int lcd_send_data_4bit(LCD* lcd, char rs, char data) 
{
    int err;
    char data4b; 

    rs = rs? 0x20 : 0x00 ; 

    // First nibble - MSbs
    data4b = (data >> 4); 
    if ( (err = lcd_data_latch(lcd, rs, data4b)) < 0 )
	return err;     
    
    // Second nibble - MSbs
    data4b = data & 0x0f; 
    if ( (err = lcd_data_latch(lcd, rs, data4b)) < 0 )
	return err;     

    return 0; 
}

/* 
 *   lcd_send_nibble: Send only a nibble( for special commands )
 */ 

int lcd_send_nibble(LCD* lcd, char rs, char data) 
{
    int err; 
    char data4b; 

    rs = rs? 0x20 : 0x00 ; 

    // First nibble - MSbs
    data4b = data & 0x0f; 

    if ( (err = lcd_data_latch(lcd, rs, data4b)) < 0 )
	return err;     

    return 0; 
}

/** 
 *  High level simplifications for using the LCD
 **/

inline int lcd_on(LCD* lcd)
{
    return lcd_send_data_4bit(lcd, 0, LCD_ON); 
}

inline int lcd_off(LCD* lcd)
{
    return lcd_send_data_4bit(lcd, 0, LCD_OFF); 
}

inline int lcd_clear(LCD* lcd)
{
  int err; 
  if(( err = lcd_send_data_4bit(lcd, 0, LCD_CLR)) < 0)
      return err; 
  
  __usleep(1640);
  return 0; 
}

/* 
 *   lcd_init: Inits the lcd in 4bit mode
 */ 

int lcd_init(LCD* lcd)
{
    int i,err;
    util_pdbg(DBG_INFO, "Initializing LCD:\n");

    // delay 15ms after processor reset
    //     __usleep (15000); // this is not necessary since the LCD will always init far too much time after the reset

    err = gpio_init(&(lcd->gpio), LCD16X2_BASE, LCD16X2_END, 
			          LCD16X2_NUM_OF_CHAN , GPIO_FLAGS_OUTPUT, 0, 
			          NULL, 0); 
    if (err < 0 ){
	    util_pdbg(DBG_INFO, "Cannot init LCD's GPIO device. Error \n",err);
	    return err; 	
    }
    

    UTIL_MUTEX_CREATE("LCD",&(lcd->mutex),NULL);
    
    // NOTE: output Display Set 3 times with 50ms delay. This workaround might be required sometimes for initialization 
    for (i=0; i < 3; i++){
	if(( err = lcd_send_nibble(lcd, 0, LCD_SET8 >> 4)) < 0 ){
	    util_pdbg(DBG_INFO, "Cannot init LCD. Error \n",err);
	    return err; 
	}
	__usleep (50000); 
    }

    // now force display to 4 bit mode
    if(( err = lcd_send_nibble(lcd, 0, LCD_SET >> 4)) < 0)
	return err;     
    __usleep(50000);
    
    // finally send out full display set command
    if(( err = lcd_send_data_4bit(lcd, 0, LCD_SET)) < 0 )
	return err; 
    
    __usleep(50);

    if(( err = lcd_off(lcd)) < 0)
	return err; 
    
    if(( err = lcd_clear(lcd)) < 0)
	return err;
    
    if(( err = lcd_on(lcd)) < 0)
	return err; 

    // set display mode
    if(( err = lcd_send_data_4bit(lcd, 0, LCD_MD)) < 0)
	return err; 
    
    // set display address
    if(( err = lcd_send_data_4bit(lcd, 0, LCD_ADR)) < 0)
	return err; 

    return 0; 
}

int lcd_clean(LCD* lcd)
{
    int err; 
    //Clear lcd?
    lcd_clear(lcd);
    
    UTIL_MUTEX_DELETE("LCD",&(lcd->mutex));
    
    return 0; 
}

int lcd_print(LCD* lcd, const char *msg)
{
    int err; 
    int i= 0 ; 

    if(( err = lcd_send_data_4bit(lcd, 0, LCD_ADR)) < 0)
	return err; 
    
    for( i = 0 ; i < 16 && *msg != 0 ; i++ ) {
        if(( err = lcd_send_data_4bit( lcd, 1 , *msg)) < 0 )
	    return err; 
        msg++;
    }
    // we jump to the position 40
    lcd_send_data_4bit(lcd, 0, LCD_ADR2); 
    for( i = 0 ; i < 16 && *msg != 0 ; i++ ) {
        if(( err = lcd_send_data_4bit(lcd, 1 , *msg)) < 0 )
	    return err; 
        msg++;
    }
    
    return 0; 
}
