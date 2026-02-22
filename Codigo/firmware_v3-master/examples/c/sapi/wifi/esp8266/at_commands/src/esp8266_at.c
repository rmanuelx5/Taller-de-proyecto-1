// src/esp8266_at.c
#include "sapi.h"
#include "esp8266_at.h"
#include <string.h>
#include <stdio.h>

// -------------------- Estado interno del módulo --------------------

static esp_cfg_t g_cfg;

// buffers internos
static char g_esp_buf[1024];

// Request assembler (+IPD -> payload)
static char g_reqBuf[512];
static int  g_reqLen  = 0;
static char g_curConn = ' ';
static int  g_curNeed = 0;

// -------------------- Helpers internos --------------------


// Acá asumimos que uart_dbg SIEMPRE es un UART válido, y controlás qué se imprime con flags.
static void dbg_write(const char* s)
{
   uartWriteString(g_cfg.uart_dbg, s);
}

static void dbg_write_ch(uint8_t c)
{
   uartWriteByte(g_cfg.uart_dbg, c);
}

static bool_t esp_send_cmd(const char* cmd, const char* wait_for, int timeout_ms)
{
   memset(g_esp_buf, 0, sizeof(g_esp_buf));
   uartWriteString(g_cfg.uart_esp, cmd);

   int buffer_index = 0;
   tick_t start_time = tickRead();
   g_esp_buf[0] = '\0';

   while ((tickRead() - start_time) < (tick_t)timeout_ms) {
      uint8_t byte_leido;
      if (uartReadByte(g_cfg.uart_esp, &byte_leido)) {

         if(g_cfg.debug_echo) dbg_write_ch(byte_leido);

         if (buffer_index < ((int)sizeof(g_esp_buf) - 1)) {
            g_esp_buf[buffer_index++] = (char)byte_leido;
            g_esp_buf[buffer_index] = '\0';
         } else {
            int half = (int)sizeof(g_esp_buf) / 2;
            memcpy(g_esp_buf, g_esp_buf + half, half);
            memset(g_esp_buf + half, 0, half);
            buffer_index = half;
            g_esp_buf[buffer_index++] = (char)byte_leido;
            g_esp_buf[buffer_index] = '\0';
         }

         if (strstr(g_esp_buf, wait_for) != NULL) return TRUE;
      }
   }

   dbg_write("\r\n--- TIMEOUT ESPERANDO RESPUESTA ---\r\n");
   return FALSE;
}

static int ipd_parse_header(const char* s, char* outConn, int* outLen)
{
   // Espera algo tipo "+IPD,0,423:"
   const char* p = strstr(s, "+IPD,");
   if(!p) return -1;

   p += 5;

   // conn id (0..4 típico en AT con CIPMUX=1)
   if(*p < '0' || *p > '4') return -1;
   *outConn = *p;

   p = strchr(p, ',');
   if(!p) return -1;
   p++;

   int len = 0;
   while(*p >= '0' && *p <= '9') {
      len = len*10 + (*p - '0');
      p++;
   }
   if(*p != ':') return -1;

   *outLen = len;
   return (int)(p - s) + 1;
}

static bool_t http_get_param_u8(const char* req, const char* key, uint8_t* out)
{
   char pat[24];
   snprintf(pat, sizeof(pat), "%s=", key);

   const char* q = strstr(req, "?");
   if(!q) return FALSE;

   const char* p = strstr(q, pat);
   if(!p) {
      snprintf(pat, sizeof(pat), "&%s=", key);
      p = strstr(q, pat);
      if(!p) return FALSE;
      p += 1; // saltear '&'
   }

   p += (int)strlen(key) + 1; // saltear "key="

   int v = 0;
   bool_t any = FALSE;
   while(*p >= '0' && *p <= '9') {
      any = TRUE;
      v = v*10 + (*p - '0');
      p++;
   }
   if(!any) return FALSE;
   if(v < 0 || v > 255) return FALSE;

   *out = (uint8_t)v;
   return TRUE;
}

static void parse_http_to_event(char conn, const char* req, esp_event_t* ev)
{
   // default
   ev->type    = ESP_EVT_NONE;
   ev->conn    = conn;
   ev->has_hm  = FALSE;
   ev->h       = 0;
   ev->m       = 0;
   ev->raw_req = req;

   if(strstr(req, "GET /ping") != NULL) {
      ev->type = ESP_EVT_PING;
      return;
   }

   if(strstr(req, "GET /dispensar") != NULL) {
      ev->type = ESP_EVT_DISPENSAR;
      return;
   }

   if(strstr(req, "GET /programar") != NULL) {
      uint8_t h, m;
      ev->type = ESP_EVT_PROGRAMAR;
      if(http_get_param_u8(req, "hora", &h) && http_get_param_u8(req, "min", &m)) {
         ev->has_hm = TRUE;
         ev->h = h;
         ev->m = m;
      }
      return;
   }

   if(strstr(req, "GET /cada") != NULL) {
      uint8_t h, m;
      ev->type = ESP_EVT_CADA;
      if(http_get_param_u8(req, "h", &h) && http_get_param_u8(req, "min", &m)) {
         ev->has_hm = TRUE;
         ev->h = h;
         ev->m = m;
      }
      return;
   }
}

// -------------------- API pública --------------------

void esp_setup_uarts(const esp_cfg_t* cfg)
{
   if(!cfg) return;
   g_cfg = *cfg;

   // Config por defecto (asumimos 115200)
   uartConfig(g_cfg.uart_dbg, 115200);
   uartConfig(g_cfg.uart_esp, 115200);
}

bool_t esp_init_ap_server(const esp_cfg_t* cfg,
                          const char* ssid,
                          const char* pass,
                          int chan,
                          int encr,
                          int delay_ms)
{
   if(cfg) g_cfg = *cfg;

   // limpiar RX del ESP
   uint8_t dummy;
   while(uartReadByte(g_cfg.uart_esp, &dummy)) {}

   // probar AT
   bool_t comm_ok = FALSE;
   for(int i=0;i<3;i++){
      if(esp_send_cmd("AT\r\n", "OK", delay_ms)) { comm_ok = TRUE; break; }
      delay(500);
   }
   if(!comm_ok) {
      dbg_write("ADVERTENCIA: ESP no responde a AT (continua)\r\n");
   }

   // modo AP
   if(!esp_send_cmd("AT+CWMODE=2\r\n", "OK", delay_ms)) {
      dbg_write("ERROR: AT+CWMODE fallo\r\n");
      return FALSE;
   }

   // crear red
   char cmd[160];
   snprintf(cmd, sizeof(cmd),
            "AT+CWSAP=\"%s\",\"%s\",%d,%d\r\n",
            ssid ? ssid : "test",
            pass ? pass : "0123456789",
            chan, encr);

   if(!esp_send_cmd(cmd, "OK", delay_ms)) {
      dbg_write("ERROR: AT+CWSAP fallo\r\n");
      return FALSE;
   }

   // múltiples conexiones
   if(!esp_send_cmd("AT+CIPMUX=1\r\n", "OK", delay_ms)) {
      dbg_write("ERROR: AT+CIPMUX fallo\r\n");
      return FALSE;
   }

   // server 80
   if(!esp_send_cmd("AT+CIPSERVER=1,80\r\n", "OK", delay_ms)) {
      dbg_write("ERROR: AT+CIPSERVER fallo\r\n");
      return FALSE;
   }

   return TRUE;
}

void esp_send_http_ok(char connection_id)
{
   const char* http_response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n"
      "Connection: close\r\n\r\n";

   char cmd_buffer[64];
   int total_len = (int)strlen(http_response);
   snprintf(cmd_buffer, sizeof(cmd_buffer),
            "AT+CIPSEND=%c,%d\r\n",
            connection_id, total_len);

   if(esp_send_cmd(cmd_buffer, ">", 2000)) {
      uartWriteString(g_cfg.uart_esp, http_response);
   }
}

void esp_close_conn(char connection_id)
{
   char cmd[32];
   snprintf(cmd, sizeof(cmd), "AT+CIPCLOSE=%c\r\n", connection_id);
   delay(100);
   uartWriteString(g_cfg.uart_esp, cmd);
}

bool_t esp_poll(esp_event_t* out_evt)
{
   if(!out_evt) return FALSE;

   // default
   out_evt->type    = ESP_EVT_NONE;
   out_evt->conn    = ' ';
   out_evt->has_hm  = FALSE;
   out_evt->h       = 0;
   out_evt->m       = 0;
   out_evt->raw_req = NULL;

   uint8_t c;

   // puente opcional USB -> ESP (tecleo manual)
   if(g_cfg.usb_bridge && uartReadByte(g_cfg.uart_dbg, &c)) {
      uartWriteByte(g_cfg.uart_esp, c);
   }

   // Consumir lo disponible del ESP
   while(uartReadByte(g_cfg.uart_esp, &c)) {

      if(g_cfg.debug_echo) dbg_write_ch(c);

      if(g_curNeed == 0) {
         // acumulamos header hasta detectar +IPD,...:
         if(g_reqLen < (int)sizeof(g_reqBuf)-1) {
            g_reqBuf[g_reqLen++] = (char)c;
            g_reqBuf[g_reqLen] = '\0';
         } else {
            g_reqLen = 0;
            g_reqBuf[0] = '\0';
         }

         char conn;
         int len;
         int payloadStart = ipd_parse_header(g_reqBuf, &conn, &len);
         if(payloadStart >= 0) {
            g_curConn = conn;
            g_curNeed = len;

            // compactar: quedarnos con lo que ya vino después del ':'
            int have = g_reqLen - payloadStart;
            if(have > 0) {
               memmove(g_reqBuf, g_reqBuf + payloadStart, have);
               g_reqLen = have;
               g_reqBuf[g_reqLen] = '\0';
               g_curNeed -= have;
            } else {
               g_reqLen = 0;
               g_reqBuf[0] = '\0';
            }
         }
      } else {
         // leyendo payload
         if(g_reqLen < (int)sizeof(g_reqBuf)-1) {
            g_reqBuf[g_reqLen++] = (char)c;
            g_reqBuf[g_reqLen] = '\0';
         }
         g_curNeed--;

         if(g_curNeed == 0) {
            // request completo -> emitir 1 evento
            if(strstr(g_reqBuf, "HTTP/") != NULL) {
               parse_http_to_event(g_curConn, g_reqBuf, out_evt);
            } else {
               out_evt->type = ESP_EVT_NONE;
            }

            // reset assembler
            g_curConn = ' ';
            g_reqLen = 0;
            g_reqBuf[0] = '\0';

            // devolvemos 1 evento por poll
            return (out_evt->type != ESP_EVT_NONE);
         }
      }
   }

   return FALSE;
}
