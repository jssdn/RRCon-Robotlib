#ifndef __GPIOMAPS_H__
#define __GPIOMAPS_H__

/* *******************************************************************************

    Project: Robotics library for the Autonomous Robotics Development Platform 
    Author:_Jorge Sánchez de Nova jssdn (mail)_(at) kth.se 
    Code: Description for the mappings of the GPIO devices

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

*  ******************************************************************************* */

// TODO: Get them in an automated manner

//Several devices are attached to the following outputs. A 16x2 LCD (7 bit), 4 LEDs (4bit), 5 Position LEDS (5bit) and a USB_RESET_SIGNAL(1bit)

#define GENERAL_OUTPUTS_BASE        0x814a0000
#define GENERAL_OUTPUTS_END         0x814affff
#define GENERAL_OUTPUTS_NUM_OF_GPIO 17
#define GENERAL_OUTPUTS_LCD_MASK    0x0007f
#define GENERAL_OUTPUTS_LED4_MASK   0x00780
#define GENERAL_OUTPUTS_LEDPOS_MASK 0x0f800
//#define GENERAL_USB_RESET_MASK    0x10000 /* Not for use */

//Several devices are attached to the following inputs. Directional buttons (5bit), an Accelerator interrupt line (1bit) and an ADC End Of Conversion signal (1bit)

#define GENERAL_INPUTS_BASE         0x81420000
#define GENERAL_INPUTS_END          0x8142ffff
#define GENERAL_INPUTS_NUM_OF_GPIO  10
#define GENERAL_INPUTS_PUSHBUT_MASK 0x0001f
#define GENERAL_INPUTS_ACC_RDY_MASK 0x00020
#define GENERAL_INPUTS_ADC_EOC_MASK 0x00040
//TODO
#define GENERAL_INPUTS_IRQ_NO 	7  
#define GENERAL_INPUTS_IRQ_PRIO 1	  


// TODO: Include bumpers into the General Inputs
// Input bumpers 

// #define BUMPERS_BASE 0x400E0000
// #define BUMPERS_END  0x400Effff

// 8bit GPIO
#define FPGA_GPIO8_BASE        0x401E0000
#define FPGA_GPIO8_END         0x401E0000
#define FPGA_GPIO8_NUM_OF_GPIO 8
#define FPGA_GPIO8_MASK	       0xff
//TODO
#define FPGA_GPIO8_IRQ_NO      8
#define FPGA_GPIO8_IRQ_PRIO    1

 	  

#endif 
