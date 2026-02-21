#ifndef _MY_ESP8266_H_
#define _MY_ESP8266_H_

#include "sapi.h"

typedef enum {
    REQ_NONE,
    REQ_DISPENSAR,
    REQ_TEST,
    REQ_UNKNOWN
} RequestType;

void myEsp8266Init(void);
bool_t myEsp8266Config(char * wifiName, char * wifiPass);
RequestType myEsp8266CheckForRequests(void);
void myEsp8266SendResponse(uint8_t connId, const char * body);

#endif
