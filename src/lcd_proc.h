#ifndef __LCD_PROC_H__
#define __LCD_PROC_H__

#define LCD_XILINX_LINE_IFACE "/proc/xilinx_lcd" /* This interface accept text as input and write it serially to the screen */
#define LCD_XILINX_CMD_IFACE  "/proc/xilinx_lcd_cmd" /* This interface accept direct commands to the LCD */
#define LCD_XILINX_L1_IFACE  "/proc/xilinx_lcd_line1" /* This interface accept direct commands to the LCD */
#define LCD_XILINX_L2_IFACE  "/proc/xilinx_lcd_line2" /* This interface accept direct commands to the LCD */

enum lcd_write_mode{ TEXT = 0x00,
                     LINE1 = 0x01,
                     LINE2 = 0x02};

int lcd_write_text(char* str, uint8_t len, enum lcd_write_mode mode);

#endif
