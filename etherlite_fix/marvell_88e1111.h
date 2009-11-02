#ifndef __MARVELL_88E1111_H__
#define __MARVELL_88E1111_H__

/** *******************************************************************************
*
*    Project: Robotics library for the Autonomous Robotics Development Platform 
*    Port by: Jorge Sánchez de Nova jssdn (mail)_(at) kth.se
*    
*    NOTE: This code is based on the Xilinx XAPP1042.
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

#define MPHY_CONTROL_REG           0
#define MPHY_CONTROL_LOOPBACK      (1 << 14)
#define MPHY_CONTROL_AUTONEG       (1 << 12)
#define MPHY_CONTROL_SPD_SEL_LSB   (1 << 13)
#define MPHY_CONTROL_SPD_SEL_MSB   (1 <<  6)
#define MPHY_CONTROL_SPD_SEL_RSVD  3
#define MPHY_CONTROL_SPD_SEL_1000  2
#define MPHY_CONTROL_SPD_SEL_100   1
#define MPHY_CONTROL_SPD_SEL_10    0
#define MPHY_CONTROL_RSTRT_AUTONEG (1 <<  9)
#define MPHY_CONTROL_DUPLEX        (1 <<  8)

#define MPHY_STATUS_REG            1
#define MPHY_STATUS_LINK           (1 <<  2)

#define MPHY_ID0_REG               2
#define MPHY_ID0_MARVELL_OUI       0x0141

#define MPHY_ID1_REG               3

#define MPHY_AUTONEG_ADV_REG       4
#define MPHY_AUTONEG_ADV_100TX_FD (1 <<  8)
#define MPHY_AUTONEG_ADV_100TX_HD (1 <<  7)
#define MPHY_AUTONEG_ADV_10TX_FD  (1 <<  6)
#define MPHY_AUTONEG_ADV_10TX_HD  (1 <<  5)

#define MPHY_1000BT_CONTROL_REG    9
#define MPHY_1000BT_CONTROL_ADV_1000BT_FD  (1 << 9)
#define MPHY_1000BT_CONTROL_ADV_1000BT_HD  (1 << 8)

#define MPHY_SPCFC_STATUS_REG         17
#define MPHY_SPCFC_STAT_SPD_SHIFT     14
#define MPHY_SPCFC_STAT_SPD_RESVD     3
#define MPHY_SPCFC_STAT_SPD_1000      2
#define MPHY_SPCFC_STAT_SPD_100       1
#define MPHY_SPCFC_STAT_SPD_10        0
#define MPHY_SPCFC_STAT_DUPLEX        (1 << 13)
#define MPHY_SPCFC_STAT_SPD_DUP_RSLVD (1 << 11)
#define MPHY_SPCFC_STAT_LINK_RT       (1 << 10)
#define MPHY_SPCFC_STAT_MDIX          (1 <<  6)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/**************************** Type Definitions *******************************/
typedef void PhyWrite_t (void *phy_inst,
                         uint32_t PhyAddress,
                         uint32_t RegisterNum,
                         uint16_t PhyData);
typedef void PhyRead_t  (void *phy_inst,
                         uint32_t PhyAddress,
                         uint32_t RegisterNum,
                         uint16_t *PhyDataPtr);

/************************** Function Prototypes ******************************/
void marvell_phy_setvectors(PhyRead_t *read_fp, PhyWrite_t *write_fp);
int  marvell_phy_link_status(void *phy_inst, uint32_t PhyAddress);
void marvell_phy_display_status(void *phy_inst, uint32_t PhyAddress);
void marvell_phy_set_1000mb_autoneg(void *phy_inst, uint32_t PhyAddress, int Enable);
int  marvell_phy_detected(void *phy_inst, uint32_t PhyAddress);

#endif
