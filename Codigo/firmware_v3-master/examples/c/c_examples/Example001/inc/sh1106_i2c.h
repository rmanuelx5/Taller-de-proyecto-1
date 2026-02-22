/*===========================================================================
 * SH1106 OLED Driver - I2C version adapted for sAPI
 * Based on original SH1106 driver, modified to use I2C instead of SPI
 * 
 * Default I2C Address: 0x3C (can be 0x3D depending on hardware)
 *===========================================================================*/

#ifndef __SH1106_I2C_H
#define __SH1106_I2C_H

#include "sapi.h"

// Screen dimensions
#define SCR_W                 128  // width
#define SCR_H                 64   // height

// SH1106 I2C Address (7-bit address)
// Common addresses: 0x3C or 0x3D
#define SH1106_I2C_ADDR       0x3C

// I2C Control bytes
#define SH1106_CTRL_CMD       0x00  // Control byte for command
#define SH1106_CTRL_DATA      0x40  // Control byte for data

// SH1106 command definitions
#define SH1106_CMD_SETMUX    0xA8  // Set multiplex ratio
#define SH1106_CMD_SETOFFS   0xD3  // Set display offset
#define SH1106_CMD_STARTLINE 0x40  // Set display start line
#define SH1106_CMD_SEG_NORM  0xA0  // Column 0 is mapped to SEG0
#define SH1106_CMD_SEG_INV   0xA1  // Column 127 is mapped to SEG0
#define SH1106_CMD_COM_NORM  0xC0  // Scan from COM0 to COM[N-1]
#define SH1106_CMD_COM_INV   0xC8  // Scan from COM[N-1] to COM0
#define SH1106_CMD_COM_HW    0xDA  // Set COM pins hardware configuration
#define SH1106_CMD_CONTRAST  0x81  // Contrast control
#define SH1106_CMD_EDON      0xA5  // Entire display ON
#define SH1106_CMD_EDOFF     0xA4  // Entire display follows RAM
#define SH1106_CMD_INV_OFF   0xA6  // Normal display
#define SH1106_CMD_INV_ON    0xA7  // Inverted display
#define SH1106_CMD_CLOCKDIV  0xD5  // Set clock divide ratio
#define SH1106_CMD_DISP_ON   0xAF  // Display ON
#define SH1106_CMD_DISP_OFF  0xAE  // Display OFF
#define SH1106_CMD_COL_LOW   0x00  // Set Lower Column Address
#define SH1106_CMD_COL_HIGH  0x10  // Set Higher Column Address
#define SH1106_CMD_PAGE_ADDR 0xB0  // Set Page Address
#define SH1106_CMD_CHARGE    0x22  // Dis-charge / Pre-charge Period

// Display orientation enumeration
enum {
	LCD_ORIENT_NORMAL = 0, // No rotation
	LCD_ORIENT_CW     = 1, // Clockwise rotation
	LCD_ORIENT_CCW    = 2, // Counter-clockwise rotation
	LCD_ORIENT_180    = 3  // 180 degrees rotation
};

// Pixel draw mode
enum {
	LCD_PSET = 0x00, // Set pixel
	LCD_PRES = 0x01, // Reset pixel
	LCD_PINV = 0x02  // Invert pixel
};

// Display state
enum {
	LCD_OFF = 0,
	LCD_ON  = 1
};

// Inversion state
enum {
	LCD_INVERT_OFF = 0,
	LCD_INVERT_ON  = 1
};

// Public variables
extern uint16_t scr_width;
extern uint16_t scr_height;
extern uint8_t LCD_PixelMode;

// Function prototypes
bool_t SH1106_Init(i2cMap_t i2cPort);
void SH1106_Contrast(uint8_t contrast);
void SH1106_SetInvert(uint8_t inv_state);
void SH1106_SetDisplayState(uint8_t disp_state);
void SH1106_Orientation(uint8_t orientation);
void SH1106_Flush(void);
void SH1106_Fill(uint8_t pattern);

void LCD_Pixel(uint8_t X, uint8_t Y, uint8_t Mode);
void LCD_HLine(uint8_t X1, uint8_t X2, uint8_t Y);
void LCD_VLine(uint8_t X, uint8_t Y1, uint8_t Y2);
void LCD_Rect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2);
void LCD_FillRect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2);
void LCD_Line(int16_t X1, int16_t Y1, int16_t X2, int16_t Y2);
void LCD_Circle(int16_t X, int16_t Y, uint8_t R);

// Text functions
uint8_t LCD_PutChar(uint8_t X, uint8_t Y, char c);
uint16_t LCD_PutStr(uint8_t X, uint8_t Y, const char *str);
uint16_t LCD_PutStrCentered(uint8_t Y, const char *str);

#endif // __SH1106_I2C_H
