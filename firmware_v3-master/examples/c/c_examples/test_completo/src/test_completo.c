#include "sapi.h"
#include "sh1106.h"
#include "font5x7.h"
#include "my_esp8266.h"

#define SERVO_N   SERVO4
#define WIFI_SSID "test"
#define WIFI_PASS "0123456789"

void updateDisplay(const char* line1, const char* line2) {
    SH1106_Fill(0x00);
    LCD_PutStr(10, 20, line1, &Font5x7);
    if(line2) LCD_PutStr(10, 30, line2, &Font5x7);
    SH1106_Flush();
}

int main(void) {
    boardConfig();
    
    // OLED Init
    SH1106_Init();
    SH1106_Fill(0x00);
    SH1106_Flush();
    
    // Servo Init
    servoConfig(0, SERVO_ENABLE);
    servoConfig(SERVO_N, SERVO_ENABLE_OUTPUT);
    servoWrite(SERVO_N, 0); // Initial position
    
    // ESP8266 Init
    myEsp8266Init();
    
    updateDisplay("Conectando WiFi...", NULL);
    
    if(myEsp8266Config(WIFI_SSID, WIFI_PASS)) {
        updateDisplay("WiFi Conectado", "Esperando...");
    } else {
        updateDisplay("Error WiFi", "Reintentar...");
        // Continue anyway or loop?
    }
    
    while(1) {
        RequestType req = myEsp8266CheckForRequests();
        
        if(req == REQ_DISPENSAR) {
            updateDisplay("Peticion:", "Dispensar");
            
            // Girar servo
            servoWrite(SERVO_N, 180);
            delay(2000);
            servoWrite(SERVO_N, 0);
            
            updateDisplay("Dispensado", "Esperando...");
        }
        else if(req == REQ_TEST) {
            updateDisplay("Peticion:", "Test Conexion");
            delay(2000);
            updateDisplay("Estado: OK", "Esperando...");
        }
        
        delay(10); // Small delay
    }
    return 0;
}
