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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

#include "gpio.h" 
#include "util.h"
#include "mii-bitbang.h"

/*
 * mii_delay:
 * Pause a bit.
 */
static inline void mii_delay (void)
{
    /*
     * Target MDC Frequency of 1/(2*1us) -- 500KHz 
     */
    __usleep(1);
}

/*
 * mdio_mode_output:
 * set GPIO to OUTPUT
 */

static inline void mdio_mode_output(GPIO *mii_gpio)
{
    uint32_t dir;
    
    gpio_get_dir(mii_gpio, ~0 , 0 , &dir);
    dir &= ~MDC_MDIO_MDIO_BIT;

    /* Set the direction for the MDIO signal to be output */
    gpio_set_dir(mii_gpio, ~0 , 0 ,dir);
}

/*
 * mdio_mode_input:
 * set GPIO to INPUT
 */
static inline void mdio_mode_input(GPIO *mii_gpio)
{
    uint32_t dir;
    
    gpio_get_dir(mii_gpio, ~0 , 0 , &dir );    
    dir |= MDC_MDIO_MDIO_BIT;
    printf("MDIO dir: 0x%x\n", dir);
    /* Set the direction for the MDIO signal to be input */
    gpio_set_dir(mii_gpio, ~0 , 0 ,dir);
}

/*
 * mdio_read:
 * Read the value presently driven by the PHY on the MDIO GPIO
 */
static inline int mdio_read(GPIO *mii_gpio)
{
    uint32_t data;
    
    gpio_read_data(mii_gpio, ~0 , 0 , &data );
    printf("MDIO read: 0x%x\n", data);
    return ((data & MDC_MDIO_MDIO_BIT) >> MDC_MDIO_MDIO_BIT_SHIFT);
}

/*
 * mdio_drive_bit:
 * set the GPIO to drive the appropriate bit value on the MDIO pin
 */
static inline void mdio_drive_bit(GPIO *mii_gpio, uint32_t value)
{
    if (value)
	gpio_set1_mask(mii_gpio, MDC_MDIO_MDIO_BIT, GPIO_DATA_OFFSET);
    else
	gpio_set0_mask(mii_gpio, MDC_MDIO_MDIO_BIT, GPIO_DATA_OFFSET);
}

/*
 * mdc_drive_bit:
 * Toggle the MDC GPIO to the appropriate bit.
 */
static inline void mdc_drive_bit(GPIO *mii_gpio, uint32_t value)
{
    if (value)
	gpio_set1_mask(mii_gpio, MDC_MDIO_MDC_BIT, GPIO_DATA_OFFSET);
    else
	gpio_set0_mask(mii_gpio, MDC_MDIO_MDC_BIT, GPIO_DATA_OFFSET);
}

/*
 * mdc_clk_1_0:
 * Drive the MII Data Clock 1->0
 */
static inline void mdc_clk_1_0 (GPIO *mii_gpio)
{
    mii_delay();
    mdc_drive_bit(mii_gpio, 1);
    mii_delay();
    mdc_drive_bit(mii_gpio, 0);
}

/*
 * mii_send_address:
 * Transmit the preamble, phy address, and phy register number on the bus.
 */
static void mii_send_address (GPIO *mii_gpio, int read, uint8_t addr, uint8_t reg)
{
    int i;

    /*
     * Send a 32 bit preamble of 1's
     */
    mdio_mode_output(mii_gpio);
    mdio_drive_bit(mii_gpio, 1);             /* <<<<< */
    for (i = 0; i < MII_PREABLE_BITS; i++) {
        mdc_clk_1_0(mii_gpio);
    }

    /*
     * send the start bit (01)
     */
    mdio_drive_bit(mii_gpio, 0);             /* <<<<< */
    mdc_clk_1_0(mii_gpio);
    mdio_drive_bit(mii_gpio, 1);             /* <<<<< */
    mdc_clk_1_0(mii_gpio);
    /*
     *  send the opcode: read (10) write (10)
     */
    mdio_drive_bit(mii_gpio, read);          /* <<<<< */
    mdc_clk_1_0(mii_gpio);
    mdio_drive_bit(mii_gpio, !read);         /* <<<<< */
    mdc_clk_1_0(mii_gpio);

    /*
     * send the PHY address
     */
    for (i = 0; i < MII_5ADDRESS_BITS; i++) {
        if (addr & FIFTH_BIT_0x10) { 
             mdio_drive_bit(mii_gpio, 1);     /* <<<<< */
        } else {
             mdio_drive_bit(mii_gpio, 0);     /* <<<<< */
        }
        mdc_clk_1_0(mii_gpio);

        addr <<= 1;
    }

    /*
     * send the register address
     */
    for (i = 0; i < MII_5ADDRESS_BITS; i++) {
        if (reg & FIFTH_BIT_0x10) {
            mdio_drive_bit(mii_gpio, 1);      /* <<<<< */
        } else {
            mdio_drive_bit(mii_gpio, 0);      /* <<<<< */
        }
        mdc_clk_1_0(mii_gpio);

        reg <<= 1;
    }
}

/*
 * MiiGpio_PhyRead:
 * Read from an MII PHY register
*/
void MiiGpio_PhyRead (GPIO *mii_gpio, uint32_t PhyAddress, uint32_t RegisterNum, uint16_t *PhyDataPtr)
{
    uint16_t regval;
    uint16_t i;

    if (PhyDataPtr == NULL) {
        return;
    }
    *PhyDataPtr = MII_DATA_INVALID;
    if (mii_gpio == NULL) {
        return;
    }

    /*
     * Bang the PHY address and PHY register on the bus.
     */
    mii_send_address(mii_gpio, MII_READ, PhyAddress, RegisterNum);

    /*
     * Set GPIO mode to read
     */
    mdio_mode_input(mii_gpio);
    mdc_clk_1_0(mii_gpio);

    /*
     * check the turnaround bit
     */
    if (mdio_read(mii_gpio) != 0) {              /* >>>>>>>> */
        printf("ERROR: PHY not driving turnaround bit low.\n\r");
        *PhyDataPtr = MII_DATA_INVALID;
        return;
    }

    /*
     * read 16 bits of register data, MSB first
     */
    regval = 0;
    for (i = 0; i < MII_16REGISTER_BITS; i++) {
        mdc_clk_1_0(mii_gpio);

        regval <<= 1;
        regval |= mdio_read(mii_gpio);            /* >>>>>>>> */
    }

    mdc_clk_1_0(mii_gpio);

    *PhyDataPtr = regval;
}

/*
 * MiiGpio_PhyWrite:
 * Write to an MII PHY register
 */
void MiiGpio_PhyWrite (GPIO *mii_gpio, uint32_t PhyAddress, uint32_t RegisterNum, uint16_t PhyData)
{
    int i;

    if (mii_gpio == NULL) {
        return;
    }

    /*
     * Bang the PHY address and PHY register on the bus.
     */
    mii_send_address(mii_gpio, MII_WRITE, PhyAddress, RegisterNum);

    /* send the turnaround (10) */
    mdio_drive_bit(mii_gpio, 1);            /* <<<<< */
    mdc_clk_1_0(mii_gpio);
    mdio_drive_bit(mii_gpio, 0);            /* <<<<< */
    mdc_clk_1_0(mii_gpio);

    /*
     * write 16 bits of register data, MSB first
     */
    for (i = 0; i < MII_16REGISTER_BITS; i++) {
        if (PhyData & MSB_16BITS_0x8000) {
            mdio_drive_bit(mii_gpio, 1);    /* <<<<< */
        } else {
            mdio_drive_bit(mii_gpio, 0);    /* <<<<< */
        }
        mdc_clk_1_0(mii_gpio);

    	PhyData <<= 1;
    }

    mdio_mode_input(mii_gpio);
    mdc_clk_1_0(mii_gpio);
}

// TODO!
/*
 * MiiGpio_Init:
 * Initialize GPIOs connected to PHY MDC and MDIO pins
 */
int MiiGpio_Init(GPIO *mii_gpio)
{    
    uint32_t Data;
    int err;

    if (mii_gpio == NULL) {
         return -ENODEV;
    }

    /*
     * Initialize the GPIO component
     */
    
    //TODO: CHANGE!
    err = gpio_init(mii_gpio,  GPIO_MDCMDIO_BASE, GPIO_MDCMDIO_END, 
			       1 , GPIO_FLAGS_OUTPUT | GPIO_FLAGS_INPUT, 0, 
			       NULL, 0); 

    if( err  < 0 )
	return err;

    /*
     * Set the direction for MDC signals to be output
     */
//     GPIO_SetDataDirection(mii_gpio, MDC_MDIO_GPIO_CHANNEL, MDC_MDIO_MDIO_BIT);
    gpio_set_dir(mii_gpio,~0,0,MDC_MDIO_MDIO_BIT);
    return 0; 
}
