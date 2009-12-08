/**
    @file lcd.c
    
    @section DESCRIPTION    
    
    Robotics library for the Autonomous Robotics Development Platform  
    
    @brief High Level 16x2 LCD functions for a 4bit interface
    
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
    
    @note See application note AN2381 ST 

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
#include <errno.h>
//Xenomai
#include <native/mutex.h>
//--
#include "lcd.h"
#include "dev_mmaps_parms.h"
#include "gpio.h"
#include "util.h"

/**
* @brief Send data in two times in 4 bit mode
*
* @param lcd LCD device to use
* @return 0 on success. Otherwise error. 
*
* Send data in two times in 4 bit mode
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

static int lcd_data_latch(LCD* lcd, char rs, char data4b)
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

/**
* @brief Send data in two times in 4 bit mode
*
* @param lcd LCD device to use
* @return 0 on success. Otherwise error. 
*
* Send data in two times in 4 bit mode
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

int lcd_send_data_4bit(LCD* lcd, char rs, char data) 
{
    int err,err2;
    char data4b; 

    rs = rs? 0x20 : 0x00 ; 

    UTIL_MUTEX_ACQUIRE("LCD:" ,&(lcd->mutex),TM_INFINITE);
    
    // First nibble - MSbs
    data4b = (data >> 4); 
    lcd_data_latch(lcd, rs, data4b);
    
    // Second nibble - MSbs - Just need to get the error here
    data4b = data & 0x0f; 
    err2 = lcd_data_latch(lcd, rs, data4b);

    UTIL_MUTEX_RELEASE("LCD:" ,&(lcd->mutex));
    
    return err2; 
}

/**
* @brief Send 4bit command
*
* @param lcd LCD device to use
* @return 0 on success. Otherwise error. 
*
* Send only a nibble( for special commands )
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

/* 
 *   lcd_send_nibble: Send only a nibble( for special commands )
 */ 

int lcd_send_nibble(LCD* lcd, char rs, char data) 
{
    int err,err2; 
    char data4b; 

    rs = rs? 0x20 : 0x00 ; 

    // First nibble - MSbs
    data4b = data & 0x0f; 
    UTIL_MUTEX_ACQUIRE("LCD:" ,&(lcd->mutex),TM_INFINITE);
    
    err2 = lcd_data_latch(lcd, rs, data4b);	

    UTIL_MUTEX_RELEASE("LCD:" ,&(lcd->mutex));

    return err2; 
}

/**
* @brief Turns on the LCD screen
*
* @param lcd LCD device to use
* @return 0 on success. Otherwise error. 
*
* High level simplifications for using the LCD
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int lcd_on(LCD* lcd)
{
    return lcd_send_data_4bit(lcd, 0, LCD_ON); 
}

/**
* @brief Turns off the LCD screen
*
* @param lcd LCD device to use
* @return 0 on success. Otherwise error. 
*
* High level simplifications for using the LCD
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int lcd_off(LCD* lcd)
{
    return lcd_send_data_4bit(lcd, 0, LCD_OFF); 
}

/**
* @brief Clears the LCD screen
*
* @param lcd LCD device to use
* @return 0 on success. Otherwise error. 
*
* High level simplifications for using the LCD
*
* @note This function is \b thread-safe.
* @note This function is \b blocking. 
*
*/

inline int lcd_clear(LCD* lcd)
{
  int err; 
  if(( err = lcd_send_data_4bit(lcd, 0, LCD_CLR)) < 0)
      return err; 
  
  __usleep(1640);
  return 0; 
}

/**
* @brief Inits the lcd in 4bit mode
*
* @param lcd LCD device to use
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
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
    
    /* Set all outputs */
    gpio_set_dir(&(lcd->gpio),~0,0,0); 
    
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

/**
* @brief Cleans the lcd device structure 
*
* @param lcd LCD device to clean
* @return 0 on success. Otherwise error. 
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

int lcd_clean(LCD* lcd)
{
    int err; 
    //Clear lcd?
    lcd_clear(lcd);
    
    UTIL_MUTEX_DELETE("LCD",&(lcd->mutex));
    
    return 0; 
}

/**
* @brief Writes a string into the 16x2 LCD 
*
* @param lcd LCD device to use
* @param msg Message to send 
* @return 0 on success. Otherwise error. 
*
* Writes a string into the 16x2 LCD by splitting it between the two lines
*
* @note This function is \b NOT thread-safe. The user should guarantee somewhere else that is not called in several instances
*       for the same resource. 
*
*/

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

