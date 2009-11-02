#ifndef __MII_BITBANG_H__
#define __MII_BITBANG_H__

/** *******************************************************************************
*
*    Project: Robotics library for the Autonomous Robotics Development Platform 
*    Port by: Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
*    
*    NOTE: This code is based on the Xilinx XAPP1042 which, as noted by Xilinx, is based in Linux Kernel's 
*	   drivers/net/fs_enet by Vitaly Bordug <vbordug@ru.mvista.com> and Pantelis Antoniou <panto@intracom.gr>
* 	 
* 	Description:
* 		Software to access PHY registers in a system where the serial control bus
* 		signals (MDC, MDIO) are connected to GPIO. 
*
*     Xilinx disclaimer below apply.
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*     SOLELY FOR USE IN DEVELOPING PROGRAMS AND SOLUTIONS FOR
*     XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION
*     AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION
*     OR STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS
*     IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*     FOR A PARTICULAR PURPOSE.
*
*     (c) Copyright 2008 Xilinx, Inc.
*     All rights reserved.
*
******************************************************************************* **/

#define GPIO_MDCMDIO_BASE 0x81000000
#define GPIO_MDCMDIO_END 0x8100FFFF

/*
 * Which GPIO bit is MDC and which is MDIO are determined by:
 *
 * From system.mhs:
 * PORT fpga_0_MII_MDC_MDIO_GPIO_IO_pin = fpga_0_MII_MDC_MDIO_GPIO_IO,
 *                                        DIR = IO, VEC = [1:0]
 *
 * From ML403 data/system.ucf:
 * # MDIO
 * Net fpga_0_MII_MDC_MDIO_GPIO_IO_pin<1> LOC = G4;
 * # MDC
 * Net fpga_0_MII_MDC_MDIO_GPIO_IO_pin<0> LOC = D1;
 */
#define MDC_MDIO_DEVICE_ID     XPAR_MII_MDC_MDIO_DEVICE_ID
#define MDC_MDIO_GPIO_CHANNEL  1

#define MDC_MDIO_MDIO_BIT       2
#define MDC_MDIO_MDIO_BIT_SHIFT 1
#define MDC_MDIO_MDC_BIT        1
#define MDC_MDIO_MDC_BIT_SHIFT  0

#define MII_READ                1
#define MII_WRITE               0
#define MII_PREABLE_BITS        32
#define MII_5ADDRESS_BITS       5
#define MII_16REGISTER_BITS     16
#define MII_DATA_INVALID        0xFFFF
#define FIFTH_BIT_0x10          0x10
#define MSB_16BITS_0x8000       0x8000

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "gpio.h" 

void MiiGpio_PhyRead(GPIO *mii_gpio, uint32_t PhyAddress, uint32_t RegisterNum, uint16_t *PhyDataPtr);

void MiiGpio_PhyWrite(GPIO *mii_gpio, uint32_t PhyAddress, uint32_t RegisterNum, uint16_t PhyData);

int MiiGpio_Init(GPIO *mii_gpio);

#endif
