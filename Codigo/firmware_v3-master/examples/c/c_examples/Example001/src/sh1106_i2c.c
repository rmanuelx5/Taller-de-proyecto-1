/*===========================================================================
 * SH1106 OLED Driver - I2C Implementation for sAPI
 *===========================================================================*/

#include "sh1106_i2c.h"
#include "font5x7.h"
#include <string.h>

// Screen dimensions
uint16_t scr_width  = SCR_W;
uint16_t scr_height = SCR_H;

// Pixel drawing mode
uint8_t LCD_PixelMode = LCD_PSET;

// Display image orientation
static uint8_t scr_orientation = LCD_ORIENT_NORMAL;

// I2C port being used
static i2cMap_t i2c_port = I2C0;

// Video RAM buffer
static uint8_t vRAM[(SCR_W * SCR_H) >> 3];

// Vertical line drawing look up tables
static const uint8_t LUT_FB[] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
static const uint8_t LUT_LB[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };

/*===========================================================================*/
/* Private functions                                                         */
/*===========================================================================*/

// Send single byte command to display
static bool_t SH1106_cmd(uint8_t cmd) {
    uint8_t buffer[2];
    buffer[0] = SH1106_CTRL_CMD;  // Control byte for command
    buffer[1] = cmd;
    
    return i2cWrite(i2c_port, SH1106_I2C_ADDR, buffer, 2, TRUE);
}

// Send double byte command to display
static bool_t SH1106_cmd_double(uint8_t cmd1, uint8_t cmd2) {
    uint8_t buffer[3];
    buffer[0] = SH1106_CTRL_CMD;  // Control byte for command
    buffer[1] = cmd1;
    buffer[2] = cmd2;
    
    return i2cWrite(i2c_port, SH1106_I2C_ADDR, buffer, 3, TRUE);
}

// Send data bytes to display
static bool_t SH1106_data(uint8_t* data, uint16_t len) {
    // For I2C, we need to send control byte followed by data
    // We'll send in chunks with control byte prefix
    uint8_t buffer[33]; // 1 control byte + 32 data bytes max per chunk
    uint16_t sent = 0;
    
    while(sent < len) {
        uint16_t chunk = (len - sent > 32) ? 32 : (len - sent);
        buffer[0] = SH1106_CTRL_DATA; // Control byte for data
        memcpy(&buffer[1], &data[sent], chunk);
        
        if(!i2cWrite(i2c_port, SH1106_I2C_ADDR, buffer, chunk + 1, TRUE)) {
            return FALSE;
        }
        sent += chunk;
    }
    return TRUE;
}

/*===========================================================================*/
/* Public functions                                                          */
/*===========================================================================*/

// Initialize SH1106 display via I2C
bool_t SH1106_Init(i2cMap_t i2cPort) {
    i2c_port = i2cPort;
    
    // Initialize I2C peripheral
    if(!i2cInit(i2c_port, 100000)) { // 100kHz
        return FALSE;
    }
    
    delay(100); // Wait for display to power up
    
    // Initial display configuration
    SH1106_cmd(SH1106_CMD_DISP_OFF); // Display OFF during init
    
    // Set multiplex ratio (visible lines)
    SH1106_cmd_double(SH1106_CMD_SETMUX, 0x3F); // 64MUX
    
    // Set display offset
    SH1106_cmd_double(SH1106_CMD_SETOFFS, 0x00); // Offset: 0
    
    // Set display start line
    SH1106_cmd(SH1106_CMD_STARTLINE | 0x00); // Start line: 0
    
    // Set segment re-map (X coordinate)
    SH1106_cmd(SH1106_CMD_SEG_NORM);
    
    // Set COM output scan direction (Y coordinate)
    SH1106_cmd(SH1106_CMD_COM_NORM);
    
    // Set COM pins hardware configuration
    SH1106_cmd_double(SH1106_CMD_COM_HW, 0x12);
    
    // Set charge pump
    SH1106_cmd_double(SH1106_CMD_CHARGE, 0x00);
    
    // Set contrast control
    SH1106_cmd_double(SH1106_CMD_CONTRAST, 0x80); // Medium contrast
    
    // Additional SH1106 specific command
    SH1106_cmd(0x30);
    
    // Disable entire display ON
    SH1106_cmd(SH1106_CMD_EDOFF);
    
    // Disable display inversion
    SH1106_cmd(SH1106_CMD_INV_OFF);
    
    // Set clock divide ratio
    SH1106_cmd_double(SH1106_CMD_CLOCKDIV, 0xF0);
    
    // Display ON
    SH1106_cmd(SH1106_CMD_DISP_ON);
    
    return TRUE;
}

// Set display contrast
void SH1106_Contrast(uint8_t contrast) {
    SH1106_cmd_double(SH1106_CMD_CONTRAST, contrast);
}

// Set display pixels inversion
void SH1106_SetInvert(uint8_t inv_state) {
    SH1106_cmd(inv_state ? SH1106_CMD_INV_ON : SH1106_CMD_INV_OFF);
}

// Toggle display on/off
void SH1106_SetDisplayState(uint8_t disp_state) {
    SH1106_cmd(disp_state ? SH1106_CMD_DISP_ON : SH1106_CMD_DISP_OFF);
}

// Set display orientation
void SH1106_Orientation(uint8_t orientation) {
    switch(orientation) {
        case LCD_ORIENT_CW:
            scr_width  = SCR_H;
            scr_height = SCR_W;
            SH1106_cmd(SH1106_CMD_SEG_INV);
            SH1106_cmd(SH1106_CMD_COM_NORM);
            break;
        case LCD_ORIENT_CCW:
            scr_width  = SCR_H;
            scr_height = SCR_W;
            SH1106_cmd(SH1106_CMD_SEG_NORM);
            SH1106_cmd(SH1106_CMD_COM_INV);
            break;
        case LCD_ORIENT_180:
            scr_width  = SCR_W;
            scr_height = SCR_H;
            SH1106_cmd(SH1106_CMD_SEG_NORM);
            SH1106_cmd(SH1106_CMD_COM_NORM);
            break;
        default:
            scr_width  = SCR_W;
            scr_height = SCR_H;
            SH1106_cmd(SH1106_CMD_SEG_INV);
            SH1106_cmd(SH1106_CMD_COM_INV);
            break;
    }
    scr_orientation = orientation;
}

// Send vRAM buffer to display
void SH1106_Flush(void) {
    const uint32_t pages = SCR_H >> 3; // 8 pages for 64 pixel height
    
    for(uint32_t page = 0; page < pages; page++) {
        // Set page address
        SH1106_cmd(SH1106_CMD_PAGE_ADDR | page);
        
        // Set column address (SH1106 has 2 pixel offset)
        SH1106_cmd(SH1106_CMD_COL_LOW | 0x02);
        SH1106_cmd(SH1106_CMD_COL_HIGH | 0x00);
        
        // Send page data
        SH1106_data(&vRAM[page * SCR_W], SCR_W);
    }
}

// Fill vRAM memory with specified pattern
void SH1106_Fill(uint8_t pattern) {
    memset(vRAM, pattern, sizeof(vRAM));
}

// Set pixel in vRAM buffer
void LCD_Pixel(uint8_t X, uint8_t Y, uint8_t Mode) {
    uint32_t offset;
    uint32_t bpos;
    
    if(scr_orientation == LCD_ORIENT_CW || scr_orientation == LCD_ORIENT_CCW) {
        offset = ((X >> 3) << 7) + Y;
        bpos   = X & 0x07;
    } else {
        offset = ((Y >> 3) << 7) + X;
        bpos   = Y & 0x07;
    }
    
    if(offset >= sizeof(vRAM)) {
        return;
    }
    
    switch(Mode) {
        case LCD_PRES:
            vRAM[offset] &= ~(1 << bpos);
            break;
        case LCD_PINV:
            vRAM[offset] ^=  (1 << bpos);
            break;
        default:
            vRAM[offset] |=  (1 << bpos);
            break;
    }
}

// Internal optimized horizontal line
static void LCD_HLineInt(uint8_t X, uint8_t Y, uint8_t W) {
    uint8_t *ptr = &vRAM[((Y >> 3) << 7)] + X;
    uint8_t mask = 1 << (Y & 0x07);
    
    switch(LCD_PixelMode) {
        case LCD_PRES:
            mask = ~mask;
            while(W--) *ptr++ &= mask;
            break;
        case LCD_PINV:
            while(W--) *ptr++ ^= mask;
            break;
        default:
            while(W--) *ptr++ |= mask;
            break;
    }
}

// Internal optimized vertical line
static void LCD_VLineInt(uint8_t X, uint8_t Y, uint8_t H) {
    uint8_t *ptr = &vRAM[((Y >> 3) << 7)] + X;
    uint8_t mask, modulo;
    
    modulo = (Y & 0x07);
    if(modulo) {
        modulo = 8 - modulo;
        mask = LUT_FB[modulo];
        if(modulo > H) mask &= (0xFF >> (modulo - H));
        
        switch(LCD_PixelMode) {
            case LCD_PRES: *ptr &= ~mask; break;
            case LCD_PINV: *ptr ^=  mask; break;
            default:       *ptr |=  mask; break;
        }
        
        if(modulo > H) return;
        ptr += SCR_W;
        H -= modulo;
    }
    
    if(H > 7) {
        switch(LCD_PixelMode) {
            case LCD_PRES:
                do { *ptr = 0x00; ptr += SCR_W; H -= 8; } while(H > 7);
                break;
            case LCD_PINV:
                do { *ptr = ~(*ptr); ptr += SCR_W; H -= 8; } while(H > 7);
                break;
            default:
                do { *ptr = 0xFF; ptr += SCR_W; H -= 8; } while(H > 7);
                break;
        }
    }
    
    if(H) {
        modulo = (H & 0x07);
        mask = LUT_LB[modulo];
        switch(LCD_PixelMode) {
            case LCD_PRES: *ptr &= ~mask; break;
            case LCD_PINV: *ptr ^=  mask; break;
            default:       *ptr |=  mask; break;
        }
    }
}

// Draw horizontal line
void LCD_HLine(uint8_t X1, uint8_t X2, uint8_t Y) {
    uint8_t X, W;
    if(X1 > X2) { X = X2; W = X1 - X2; }
    else { X = X1; W = X2 - X1; }
    W++;
    
    if(scr_orientation == LCD_ORIENT_CW || scr_orientation == LCD_ORIENT_CCW) {
        LCD_VLineInt(Y, X, W);
    } else {
        LCD_HLineInt(X, Y, W);
    }
}

// Draw vertical line
void LCD_VLine(uint8_t X, uint8_t Y1, uint8_t Y2) {
    uint8_t Y, H;
    if(Y1 > Y2) { Y = Y2; H = Y1 - Y2; }
    else { Y = Y1; H = Y2 - Y1; }
    H++;
    
    if(scr_orientation == LCD_ORIENT_CW || scr_orientation == LCD_ORIENT_CCW) {
        LCD_HLineInt(Y, X, H);
    } else {
        LCD_VLineInt(X, Y, H);
    }
}

// Draw rectangle
void LCD_Rect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2) {
    LCD_HLine(X1, X2, Y1);
    LCD_HLine(X1, X2, Y2);
    LCD_VLine(X1, Y1 + 1, Y2 - 1);
    LCD_VLine(X2, Y1 + 1, Y2 - 1);
}

// Draw filled rectangle
void LCD_FillRect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2) {
    uint8_t Z, E, T, L;
    
    if(scr_orientation == LCD_ORIENT_CW || scr_orientation == LCD_ORIENT_CCW) {
        if(X1 > X2) { T = X2; L = X1 - X2; }
        else { T = X1; L = X2 - X1; }
        if(Y1 > Y2) { Z = Y1; E = Y2; }
        else { Z = Y2; E = Y1; }
    } else {
        if(Y1 > Y2) { T = Y2; L = Y1 - Y2; }
        else { T = Y1; L = Y2 - Y1; }
        if(X1 > X2) { Z = X1; E = X2; }
        else { Z = X2; E = X1; }
    }
    L++;
    
    do {
        LCD_VLineInt(Z, T, L);
    } while(Z-- > E);
}

// Draw line
void LCD_Line(int16_t X1, int16_t Y1, int16_t X2, int16_t Y2) {
    int16_t dX = X2 - X1;
    int16_t dY = Y2 - Y1;
    int16_t dXsym = (dX > 0) ? 1 : -1;
    int16_t dYsym = (dY > 0) ? 1 : -1;
    
    if(dX == 0) { LCD_VLine(X1, Y1, Y2); return; }
    if(dY == 0) { LCD_HLine(X1, X2, Y1); return; }
    
    dX *= dXsym;
    dY *= dYsym;
    int16_t dX2 = dX << 1;
    int16_t dY2 = dY << 1;
    int16_t di;
    
    if(dX >= dY) {
        di = dY2 - dX;
        while(X1 != X2) {
            LCD_Pixel(X1, Y1, LCD_PixelMode);
            X1 += dXsym;
            if(di < 0) { di += dY2; }
            else { di += dY2 - dX2; Y1 += dYsym; }
        }
    } else {
        di = dX2 - dY;
        while(Y1 != Y2) {
            LCD_Pixel(X1, Y1, LCD_PixelMode);
            Y1 += dYsym;
            if(di < 0) { di += dX2; }
            else { di += dX2 - dY2; X1 += dXsym; }
        }
    }
    LCD_Pixel(X1, Y1, LCD_PixelMode);
}

// Draw circle
void LCD_Circle(int16_t Xc, int16_t Yc, uint8_t R) {
    int16_t err = 1 - R;
    int16_t dx = 0;
    int16_t dy = -2 * R;
    int16_t x = 0;
    int16_t y = R;
    int16_t sh = scr_height - 1;
    int16_t sw = scr_width - 1;
    
    while(x < y) {
        if(err >= 0) {
            dy += 2;
            err += dy;
            y--;
        }
        dx += 2;
        err += dx + 1;
        x++;
        
        // Draw eight pixels of each octant
        if(Xc + x < sw) {
            if(Yc + y < sh) LCD_Pixel(Xc + x, Yc + y, LCD_PixelMode);
            if(Yc - y > -1) LCD_Pixel(Xc + x, Yc - y, LCD_PixelMode);
        }
        if(Xc - x > -1) {
            if(Yc + y < sh) LCD_Pixel(Xc - x, Yc + y, LCD_PixelMode);
            if(Yc - y > -1) LCD_Pixel(Xc - x, Yc - y, LCD_PixelMode);
        }
        if(Xc + y < sw) {
            if(Yc + x < sh) LCD_Pixel(Xc + y, Yc + x, LCD_PixelMode);
            if(Yc - x > -1) LCD_Pixel(Xc + y, Yc - x, LCD_PixelMode);
        }
        if(Xc - y > -1) {
            if(Yc + x < sh) LCD_Pixel(Xc - y, Yc + x, LCD_PixelMode);
            if(Yc - x > -1) LCD_Pixel(Xc - y, Yc - x, LCD_PixelMode);
        }
    }
    
    // Vertical and horizontal points
    if(Xc + R < sw) LCD_Pixel(Xc + R, Yc, LCD_PixelMode);
    if(Xc - R > -1) LCD_Pixel(Xc - R, Yc, LCD_PixelMode);
    if(Yc + R < sh) LCD_Pixel(Xc, Yc + R, LCD_PixelMode);
    if(Yc - R > -1) LCD_Pixel(Xc, Yc - R, LCD_PixelMode);
}

// Draw a single character
// Returns the width of the character drawn
uint8_t LCD_PutChar(uint8_t X, uint8_t Y, char c) {
    uint8_t i, j;
    const uint8_t *char_data;
    
    // Check if character is printable
    if(c < 32 || c > 126) {
        c = '?'; // Replace unprintable with '?'
    }
    
    // Get pointer to character data
    char_data = &font5x7.data[(c - 32) * 5];
    
    // Draw character
    for(i = 0; i < 5; i++) {
        uint8_t line = char_data[i];
        for(j = 0; j < 7; j++) {
            if(line & 0x01) {
                LCD_Pixel(X + i, Y + j, LCD_PixelMode);
            }
            line >>= 1;
        }
    }
    
    return 6; // Character width + 1 pixel spacing
}

// Draw a string
// Returns the width of the string in pixels
uint16_t LCD_PutStr(uint8_t X, uint8_t Y, const char *str) {
    uint8_t start_x = X;
    
    while(*str) {
        X += LCD_PutChar(X, Y, *str);
        str++;
        
        // Stop if we've gone off screen
        if(X >= scr_width) break;
    }
    
    return X - start_x;
}

// Draw a string centered horizontally
// Returns the width of the string in pixels
uint16_t LCD_PutStrCentered(uint8_t Y, const char *str) {
    // Calculate string width
    uint16_t str_width = 0;
    const char *p = str;
    while(*p) {
        str_width += 6; // 5 pixels + 1 space
        p++;
    }
    if(str_width > 0) str_width -= 1; // Remove last space
    
    // Calculate centered X position
    uint8_t X = (scr_width - str_width) / 2;
    
    return LCD_PutStr(X, Y, str);
}
