#include "sapi.h"
#include "sh1106.h"
#include "font5x7.h"

int main(void) {
    // Initialize board
    boardConfig();

    // Initialize SH1106 OLED display
    SH1106_Init();

    // Clear display buffer
    SH1106_Fill(0x00);

    // Draw text
    // Centered roughly: 128 width. "Dispenser" is 9 chars * 6 = 54 pixels.
    // "Automático" is 10 chars * 6 = 60 pixels.
    // 5x7 font width is 5, plus 1 pixel space = 6.
    
    LCD_PutStr(37, 20, "Dispenser", &Font5x7);
    LCD_PutStr(34, 30, "Automatico", &Font5x7); // Using "Automatico" to avoid encoding issues for now, or I can try to map it if I had the char.

    // Flush buffer to display
    SH1106_Flush();

    while (1) {
        delay(1000);
    }

    return 0;
}
