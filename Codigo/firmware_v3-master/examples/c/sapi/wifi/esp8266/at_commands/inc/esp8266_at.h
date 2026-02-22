// inc/esp8266_at.h
#ifndef ESP8266_AT_H
#define ESP8266_AT_H

#include "sapi.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
   ESP_EVT_NONE = 0,
   ESP_EVT_PING,
   ESP_EVT_DISPENSAR,
   ESP_EVT_PROGRAMAR,   // params: hora/min
   ESP_EVT_CADA         // params: h/min
} esp_evt_type_t;

typedef struct {
   esp_evt_type_t type;
   char           conn;   // '0'..'4'
   bool_t         has_hm;
   uint8_t        h;
   uint8_t        m;
   const char*    raw_req; // puntero a buffer interno (válido hasta próximo esp_poll)
} esp_event_t;

typedef struct {
   uartMap_t uart_esp;   // ej: UART_232
   uartMap_t uart_dbg;   // ej: UART_USB (o UART_USB para debug). Podés poner UART_USB igual
   bool_t    debug_echo; // si TRUE: eco a dbg lo que llega del ESP
   bool_t    usb_bridge; // si TRUE: puente USB->ESP (tecleo AT manual)
} esp_cfg_t;

// Inicializa UARTs según cfg (NO configura WiFi/AP).
void esp_setup_uarts(const esp_cfg_t* cfg);

// Inicializa el ESP en modo AP y servidor HTTP 80.
// Devuelve TRUE si salió todo OK (o FALSE si falló en algo crítico).
bool_t esp_init_ap_server(const esp_cfg_t* cfg,
                          const char* ssid,
                          const char* pass,
                          int chan,
                          int encr,
                          int delay_ms);

// Lee UART del ESP y, si llega un request completo, lo parsea y devuelve evento.
// Retorna TRUE si out_evt contiene un evento (incluye conn).
bool_t esp_poll(esp_event_t* out_evt);

// Envía HTTP/1.1 200 OK (sin cuerpo) por la conexión indicada.
void esp_send_http_ok(char connection_id);

// Cierra conexión (AT+CIPCLOSE=conn).
void esp_close_conn(char connection_id);

#ifdef __cplusplus
}
#endif

#endif // ESP8266_AT_H
