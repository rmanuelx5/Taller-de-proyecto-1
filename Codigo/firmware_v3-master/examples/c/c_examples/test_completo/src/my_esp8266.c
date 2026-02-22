#include "my_esp8266.h"
#include "sapi.h"
#include <string.h>
#include <stdio.h>

#define ESP_UART UART_232
// #define DEBUG_UART UART_USB

static char buffer[1024];
static uint16_t bufIdx = 0;

void myEsp8266Init(void) {
    uartConfig(ESP_UART, 115200);
}

static void sendCmd(const char* cmd) {
    uartWriteString(ESP_UART, cmd);
    uartWriteString(ESP_UART, "\r\n");
}

static bool_t waitFor(const char* expected, uint32_t timeout) {
    delay_t d;
    delayConfig(&d, timeout);
    uint8_t byte;
    uint16_t idx = 0;
    uint16_t len = strlen(expected);
    
    while(!delayRead(&d)) {
        if(uartReadByte(ESP_UART, &byte)) {
            if(byte == expected[idx]) {
                idx++;
                if(idx == len) return TRUE;
            } else {
                if(byte == expected[0]) idx = 1; else idx = 0;
            }
        }
    }
    return FALSE;
}

bool_t myEsp8266Config(char * wifiName, char * wifiPass) {
    // Flush buffer
    uint8_t dummy;
    while(uartReadByte(ESP_UART, &dummy));

    sendCmd("AT");
    if(!waitFor("OK", 1000)) return FALSE;
    
    sendCmd("AT+CWMODE=3");
    if(!waitFor("OK", 1000)) return FALSE;
    
    char cmd[100];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", wifiName, wifiPass);
    sendCmd(cmd);
    if(!waitFor("OK", 20000)) return FALSE; 
    
    sendCmd("AT+CIPMUX=1");
    if(!waitFor("OK", 1000)) return FALSE;
    
    sendCmd("AT+CIPSERVER=1,80");
    if(!waitFor("OK", 1000)) return FALSE;
    
    return TRUE;
}

void myEsp8266SendResponse(uint8_t connId, const char * body) {
    char header[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char cmd[50];
    int len = strlen(header) + strlen(body);
    
    sprintf(cmd, "AT+CIPSEND=%d,%d", connId, len);
    sendCmd(cmd);
    if(waitFor(">", 1000)) {
        uartWriteString(ESP_UART, header);
        uartWriteString(ESP_UART, body);
        waitFor("SEND OK", 2000);
    }
    
    sprintf(cmd, "AT+CIPCLOSE=%d", connId);
    sendCmd(cmd);
    // Don't wait too long for close
    waitFor("OK", 500);
}

RequestType myEsp8266CheckForRequests(void) {
    uint8_t byte;
    while(uartReadByte(ESP_UART, &byte)) {
        if(bufIdx < sizeof(buffer)-1) {
            buffer[bufIdx++] = byte;
            buffer[bufIdx] = 0;
        } else {
            bufIdx = 0; // Reset if full
        }
        
        char *ipd = strstr(buffer, "+IPD,");
        if(ipd) {
            // Check if we have enough data to see the request
            if(strstr(buffer, "GET /dispensar")) {
                int id = 0;
                sscanf(ipd, "+IPD,%d,", &id);
                
                // Clear buffer
                bufIdx = 0;
                buffer[0] = 0;
                
                myEsp8266SendResponse(id, "<html><body><h1>Dispensando...</h1></body></html>");
                return REQ_DISPENSAR;
            }
            
            if(strstr(buffer, "GET /test")) {
                int id = 0;
                sscanf(ipd, "+IPD,%d,", &id);
                
                bufIdx = 0;
                buffer[0] = 0;
                
                myEsp8266SendResponse(id, "<html><body><h1>Conexion OK</h1></body></html>");
                return REQ_TEST;
            }
            
            // If buffer is getting full and we found IPD but no match yet, 
            // we might want to clear if it's too old, but for now let it fill.
            // If we see "HTTP/1.1" but didn't match our paths, it's an unknown request.
            if(strstr(buffer, "HTTP/1.1")) {
                 int id = 0;
                sscanf(ipd, "+IPD,%d,", &id);
                bufIdx = 0;
                buffer[0] = 0;
                myEsp8266SendResponse(id, "<html><body><h1>Unknown Request</h1></body></html>");
                return REQ_UNKNOWN;
            }
        }
    }
    return REQ_NONE;
}
