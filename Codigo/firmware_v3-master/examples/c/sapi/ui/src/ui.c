/* ui.c - Contiene toda la UI (MEF + botones + scheduler + ESP events) */

#include "sapi.h"
#include <string.h>
#include <stdio.h>

#include "pantalla.h"
#include "esp8266_at.h"
#include "servo.h"
#include "ui.h"
#include "loadcell.h"

// ------------------ CONFIG UI ------------------

#define BOTON_ACEPTAR   GPIO4
#define BOTON_ABAJO     GPIO2
#define BOTON_ARRIBA    GPIO0
#define DEBOUNCE_MS     30

#define INACTIVIDAD_MS      60000
#define CLOCK_REFRESH_MS    1000

#define PORCION_MIN_G  5
#define PORCION_MAX_G  500
#define PORCION_STEP_G 5

#define DISP_STEP_MS        300     // tiempo de giro por paso
#define DISP_TIMEOUT_MS    10000   // seguridad total (10 s)
#define DISP_TOLERANCIA_G  2       // tolerancia por ruido

// ------------------ prototipos internos UI ------------------
static void obtenerHoraActual(char* buffer);

static void api_programar_una_vez(uint8_t h, uint8_t m);
static void api_cada_intervalo(uint8_t h, uint8_t m);

static void cada_set_intervalo(uint8_t h, uint8_t m);
static void cada_stop(void);

static void scheduler_check(void);
static void scheduler_disparar(const char* titulo1, const char* titulo2);

static void esp_handler(void);

static void marcarActividad(void);
static bool_t inactivo(void);
static void rtc_get_hm(uint8_t* h, uint8_t* m);

// ------------------ Tipos / Estado UI ------------------

// enum para maquina de estados principal
typedef enum {
   INICIO = 0,
   MENU,
   PANTALLA_HORA,
   OPCION_DISPENSAR,

   OPCION_DISP_EN_X,
   DISP_ENX_SET_HORA,
   DISP_ENX_SET_MIN,
   DISP_ENX_CONFIRMAR,

   OPCION_DISP_CADA_X,
   CADA_SET_HORA,
   CADA_SET_MIN,
   CADA_CONFIRMAR,

   OPCION_ELEGIR_PORCION,
   ELEGIR_GRAMOS_PORCION,
   CONFIRMAR_GRAMOS_PORCION
} estado_t;

// enum para cada item del menu
typedef enum {
   ITEM_DISPENSAR = 0,
   ITEM_DISP_EN_X,
   ITEM_DISP_CADA_X,
   ITEM_PORCION,
   ITEM_COUNT
} menu_item_t;

// enum para asociar los botones con numeros
typedef enum {
   BTN_ACEPTAR = 0,
   BTN_ABAJO,
   BTN_ARRIBA,
   BTN_CANTIDAD
} boton_id_t;

typedef struct {
   bool_t lectura;    // última lectura cruda
   bool_t estable;    // estado estable
   tick_t timer;      // timer de debounce
} boton_t;

// ------------------ Variables globales UI (solo en este módulo) ------------------

static boton_t botones[BTN_CANTIDAD] = {
   { TRUE, TRUE, 0 }, // ACEPTAR (pull-up: TRUE suelto, FALSE apretado)
   { TRUE, TRUE, 0 }, // ABAJO
   { TRUE, TRUE, 0 }  // ARRIBA
};

static bool_t disp_prog_activo = FALSE;
static bool_t disp_prog_ejecutado = FALSE; // evita re-disparar durante el mismo minuto
static uint8_t disp_prog_hora = 12;
static uint8_t disp_prog_min  = 0;

// ? Variables de edición (para NO tocar disp_prog_hora/min mientras editás)
static uint8_t disp_edit_h = 12;
static uint8_t disp_edit_m = 0;

static bool_t cada_activo = FALSE;
static tick_t cada_interval_ms = 0;
static tick_t cada_proximo_ms = 0;
static uint8_t cada_h = 6;   // default 6 horas
static uint8_t cada_m = 0;   // default 0 min

static bool_t ui_pedir_redibujar = FALSE;

static tick_t g_ultimaActividad = 0;
static tick_t g_ultimoRefreshHora = 0;

static uint16_t porcion_g = 50;   // porción por defecto en gramos

// ------------------ Helpers de tiempo ------------------

static void obtenerHoraActual(char* buffer)
{
   rtc_t rtc;
   rtcRead(&rtc);

   sprintf(buffer, "%02d:%02d:%02d", rtc.hour, rtc.min, rtc.sec);
}

static void rtc_get_hm(uint8_t* h, uint8_t* m)
{
   rtc_t rtc;
   rtcRead(&rtc);
   *h = rtc.hour;
   *m = rtc.min;
}

static void marcarActividad(void)
{
   g_ultimaActividad = tickRead();
}

static bool_t inactivo(void)
{
   return (tickRead() - g_ultimaActividad) >= INACTIVIDAD_MS;
}

// ------------------ Funcion de dispensar ------------------

void dispensarAlimento_porPeso(uint16_t porcion_g)
{
   // Obtiene el peso inicial
   int32_t ini_g = ObtenerPesoDispensador_g();
   char buf[64];
   sprintf(buf, "Peso dispensador inicial: %ld g\r\n", (long)ini_g);
   uartWriteString(UART_USB, buf);

   // Si no tiene sentido, sale
   if(ini_g < 0) return;

   int32_t objetivo_g = ini_g - (int32_t)porcion_g;

   tick_t t0 = tickRead();

   while(1) {
      // Arranca a girar el motor
      girarMotor(DISP_STEP_MS);

      int32_t actual_g = ObtenerPesoDispensador_g();
      if(actual_g >= 0) {
         if(actual_g <= (objetivo_g + DISP_TOLERANCIA_G)) {
            // Si sirvio la porcion
            sprintf(buf, "Peso dispensador terminando: %ld g\r\n", (long)actual_g);
            uartWriteString(UART_USB, buf);
            break;
         }
         sprintf(buf, "Peso dispensador actual: %ld g\r\n", (long)actual_g);
         uartWriteString(UART_USB, buf);
      }

      // timeout por si sigue girando sin parar
      if((tickRead() - t0) >= DISP_TIMEOUT_MS) break;

      delay(80);
   }

   girarMotor(0);
}

static void dispensarAlimento_simple(void)
{
   girarMotor(20000);
}

void dispensarAlimento(void)
{
   dispensarAlimento_porPeso(porcion_g);
   //dispensarAlimento_simple();
}

// ------------------ Botones ------------------

static bool_t leer_gpio_boton(boton_id_t id)
{
   switch(id) {
      case BTN_ACEPTAR: return gpioRead(BOTON_ACEPTAR);
      case BTN_ABAJO:   return gpioRead(BOTON_ABAJO);
      case BTN_ARRIBA:  return gpioRead(BOTON_ARRIBA);
      default:          return TRUE;
   }
}

static bool_t boton_evento(boton_id_t id)
{
   boton_t* b = &botones[id];
   bool_t lectura = leer_gpio_boton(id);

   if(lectura != b->lectura) {
      b->lectura = lectura;
      b->timer = tickRead();
   }

   if((tickRead() - b->timer) >= DEBOUNCE_MS) {
      if(b->estable != b->lectura) {
         b->estable = b->lectura;

         if(b->estable == FALSE) {
            return TRUE;  // evento: botón apretado
         }
      }
   }

   return FALSE;
}

// ------------------ UI Draw ------------------

static const char* menu_texto(menu_item_t item)
{
   switch(item) {
      case ITEM_DISPENSAR:   return "Dispensar alimento";
      case ITEM_DISP_EN_X:   return "Dispensar a las X";
      case ITEM_DISP_CADA_X: return "Dispensar cada X";
      case ITEM_PORCION:     return "Elegir porcion (g)";
      default:               return "";
   }
}

static void menu_dibujar(menu_item_t item)
{
   char hora[6]; // "HH:MM" + '\0'
   rtc_t rtc;
   rtcRead(&rtc);
   sprintf(hora, "%02d:%02d", rtc.hour, rtc.min);

   limpiarPantalla();
   escribirPrimeraLinea("Menu");
   escribirALaDerecha(0, hora);
   escribirSegundaLinea(menu_texto(item));
   escribirTerceraLinea("Arriba/Abajo: Mover");
   escribirCuartaLinea("OK: Entrar");
   actualizarPantalla();
}

static void ui_dibujar_set_hora(uint8_t hora, uint8_t min, bool_t editandoHora)
{
   char buf[20];

   limpiarPantalla();
   escribirPrimeraLinea("Dispensar a las:");
   sprintf(buf, "%02d:%02d", hora, min);
   escribirSegundaLinea(buf);

   escribirTerceraLinea(editandoHora ? "Editando: HORA" : "Editando: MIN");
   escribirCuartaLinea("Arr/Ab:+-  OK: sig");
   actualizarPantalla();
}

static void ui_dibujar_confirmar(uint8_t hora, uint8_t min)
{
   char buf[20];

   limpiarPantalla();
   escribirPrimeraLinea("Confirmar horario");
   sprintf(buf, "%02d:%02d", hora, min);
   escribirSegundaLinea(buf);
   escribirTerceraLinea("OK: guardar");
   escribirCuartaLinea("Arr:editar/Ab:Cancel");
   actualizarPantalla();
}

static void ui_dibujar_cada_edit(uint8_t h, uint8_t m, bool_t editHora)
{
   char buf[12];
   sprintf(buf, "%02d:%02d", h, m);

   limpiarPantalla();
   escribirPrimeraLinea("Auto cada:");
   escribirSegundaLinea(buf);
   escribirTerceraLinea(editHora ? "Editando: HORAS" : "Editando: MIN");
   escribirCuartaLinea("Arr/Ab:+-  OK:sig");
   actualizarPantalla();
}

static void ui_dibujar_cada_confirmar(uint8_t h, uint8_t m)
{
   char buf[24];

   limpiarPantalla();
   escribirPrimeraLinea("Confirmar");

   if(h == 0 && m == 0) {
      sprintf(buf, "OFF");
   } else {
      sprintf(buf, "%02d:%02d", h, m);
   }

   escribirSegundaLinea(buf);
   escribirTerceraLinea("OK: guardar");
   escribirCuartaLinea("Abajo: atras");
   actualizarPantalla();
}

static void ui_dibujar_opcion_porc(uint16_t g)
{
   char buf[24];
   sprintf(buf, "Actual: %u g", (unsigned)g);

   limpiarPantalla();
   escribirPrimeraLinea("Porcion");
   escribirSegundaLinea(buf);
   escribirTerceraLinea("OK: editar");
   escribirCuartaLinea("Abajo: atras");
   actualizarPantalla();
}

static void ui_dibujar_edit_porc(uint16_t g)
{
   char buf[24];
   sprintf(buf, "%u g", (unsigned)g);

   limpiarPantalla();
   escribirPrimeraLinea("Elegir porcion");
   escribirSegundaLinea(buf);
   escribirTerceraLinea("Arr/Ab: +/-");
   escribirCuartaLinea("OK: confirmar");
   actualizarPantalla();
}

static void ui_dibujar_confirm_porc(uint16_t g)
{
   char buf[24];
   sprintf(buf, "%u g", (unsigned)g);

   limpiarPantalla();
   escribirPrimeraLinea("Guardar porcion?");
   escribirSegundaLinea(buf);
   escribirTerceraLinea("OK: guardar");
   escribirCuartaLinea("Abajo: cancel");
   actualizarPantalla();
}

// ------------------ Scheduler ------------------

static void cada_set_intervalo(uint8_t h, uint8_t m)
{
   // OFF si 0:00
   if(h == 0 && m == 0) {
      cada_activo = FALSE;
      return;
   }

   cada_h = h;
   cada_m = m;

   uint32_t total_s = (uint32_t)h * 3600UL + (uint32_t)m * 60UL;
   cada_interval_ms = (tick_t)(total_s * 1000UL);

   cada_proximo_ms = tickRead() + cada_interval_ms;
   cada_activo = TRUE;
}

static void cada_stop(void)
{
   cada_activo = FALSE;
}

static void scheduler_disparar(const char* titulo1, const char* titulo2)
{
   limpiarPantalla();
   escribirPrimeraLinea(titulo1);
   escribirSegundaLinea(titulo2);
   escribirTerceraLinea("");
   escribirCuartaLinea("");
   actualizarPantalla();

   dispensarAlimento();
}

static void scheduler_check(void)
{
   // ---------- 1) Dispensado programado por RTC (una vez) ----------
   if(disp_prog_activo) {

      rtc_t rtc;
      rtcRead(&rtc);

      // Evitar re-disparar dentro del mismo minuto
      static uint8_t ultimo_min = 255;
      if(rtc.min != ultimo_min) {
         ultimo_min = rtc.min;
         disp_prog_ejecutado = FALSE;
      }

      if(!disp_prog_ejecutado &&
         rtc.hour == disp_prog_hora &&
         rtc.min  == disp_prog_min)
      {
         scheduler_disparar("Programado!", "Dispensando...");

         disp_prog_ejecutado = TRUE;
         disp_prog_activo = FALSE; // una sola vez
         return;
      }
   }

   // ---------- 2) Dispensado repetitivo cada X (tickRead) ----------
   if(cada_activo && cada_interval_ms > 0) {

      tick_t ahora = tickRead();

      // Comparación segura ante overflow
      if( (tick_t)(ahora - cada_proximo_ms) < (tick_t)0x80000000UL ) {

         scheduler_disparar("Auto", "Dispensando...");

         // Programar el siguiente (si estuvimos bloqueados, avanzar hasta futuro)
         do {
            cada_proximo_ms += cada_interval_ms;
         } while( (tick_t)(tickRead() - cada_proximo_ms) < (tick_t)0x80000000UL );

         return;
      }
   }
}

// ------------------ API (llamadas desde ESP events) ------------------

static void api_programar_una_vez(uint8_t h, uint8_t m)
{
   if(h > 23) h = 23;
   if(m > 59) m = 59;

   disp_prog_hora = h;
   disp_prog_min  = m;
   disp_prog_activo = TRUE;
   disp_prog_ejecutado = FALSE;

   mostrar_toast_hora("Prog. a las:", h, m, 1200);
}

static void api_cada_intervalo(uint8_t h, uint8_t m)
{
   cada_set_intervalo(h, m);

   if(h == 0 && m == 0) {
      limpiarPantalla();
      escribirPrimeraLinea("Auto cada:");
      escribirSegundaLinea("OFF");
      actualizarPantalla();
      delay(1200);
   } else {
      mostrar_toast_hora("Auto cada:", h, m, 1200);
   }
}

// ------------------ ESP handler ------------------

static void esp_handler(void)
{
   esp_event_t ev;
   if(esp_poll(&ev)) {
      switch(ev.type) {

        case ESP_EVT_PING:
          mostrar_toast("App:", "Ping OK", 1000);
          esp_send_http_ok(ev.conn);
          esp_close_conn(ev.conn);
          break;

        case ESP_EVT_DISPENSAR:
          mostrar_toast("App:", "Dispensar", 1000);
          dispensarAlimento();
          esp_send_http_ok(ev.conn);
          esp_close_conn(ev.conn);
          break;

        case ESP_EVT_PROGRAMAR:
          if(ev.has_hm) api_programar_una_vez(ev.h, ev.m);
          else mostrar_toast("App:", "Faltan params", 1000);
          esp_send_http_ok(ev.conn);
          esp_close_conn(ev.conn);
          break;

        case ESP_EVT_CADA:
          if(ev.has_hm) api_cada_intervalo(ev.h, ev.m);
          else mostrar_toast("App:", "Faltan params", 1000);
          esp_send_http_ok(ev.conn);
          esp_close_conn(ev.conn);
          break;

        default:
          break;
      }
   }
}

// ------------------ LOOP UI ------------------

void loop_ui(void)
{
   static estado_t estado = INICIO;
   static estado_t estado_prev = (estado_t)(-1);

   static menu_item_t menu_sel = ITEM_DISPENSAR;
   static menu_item_t menu_sel_prev = (menu_item_t)(-1);

   if(g_ultimaActividad == 0) {
      marcarActividad();
   }

   while(1) {

      esp_handler();
      scheduler_check();

      if(ui_pedir_redibujar) {
         estado_prev = (estado_t)(-1);
         ui_pedir_redibujar = FALSE;
      }

      bool_t ev_ok   = boton_evento(BTN_ACEPTAR);
      bool_t ev_up   = boton_evento(BTN_ARRIBA);
      bool_t ev_down = boton_evento(BTN_ABAJO);

      if(ev_ok || ev_up || ev_down) {
         marcarActividad();
      }

      // -------- ON ENTER --------
      if(estado != estado_prev) {

         limpiarPantalla();

         switch(estado) {

            case INICIO: {
               escribirPrimeraLinea("Bienvenido");
               escribirSegundaLinea("OK: Menu");

               int32_t g = ObtenerPesoDispensador_g();   // usa tu filtro median
               if(g < 0) {
                  escribirTerceraLinea("Peso: --");
               } else {
                  char buf[24];
                  sprintf(buf, "Peso: %ld g", (long)g);
                  escribirTerceraLinea(buf);
               }

               escribirCuartaLinea("");
               break;
            }

            case MENU:
               escribirPrimeraLinea("Menu");
               escribirSegundaLinea(menu_texto(menu_sel));
               escribirTerceraLinea("Arriba/Abajo: mover");
               escribirCuartaLinea("OK: Entrar");
               menu_sel_prev = menu_sel;
               break;

            case PANTALLA_HORA: {
               char hora[16];
               obtenerHoraActual(hora);
               escribirPrimeraLinea("Hora actual");
               escribirSegundaLinea(hora);
               escribirTerceraLinea("OK: Menu");
               escribirCuartaLinea("Arr/Ab: inicio");
               g_ultimoRefreshHora = tickRead();
               break;
            }

            case OPCION_DISPENSAR:
               escribirPrimeraLinea("¿Desea");
               escribirSegundaLinea("Dispensar ahora?");
               escribirTerceraLinea("OK: Confirmar");
               escribirCuartaLinea("Abajo: Atras");
               break;

            case OPCION_DISP_EN_X:
               escribirPrimeraLinea("Opcion");
               escribirSegundaLinea("A las X");
               escribirTerceraLinea("OK: Confirmar");
               escribirCuartaLinea("Abajo: Atras");
               break;

            case DISP_ENX_SET_HORA:
               ui_dibujar_set_hora(disp_edit_h, disp_edit_m, TRUE);
               break;

            case DISP_ENX_SET_MIN:
               ui_dibujar_set_hora(disp_edit_h, disp_edit_m, FALSE);
               break;

            case DISP_ENX_CONFIRMAR:
               ui_dibujar_confirmar(disp_edit_h, disp_edit_m);
               break;

            case OPCION_DISP_CADA_X:
               escribirPrimeraLinea("Dispensar cada");
               escribirSegundaLinea("OK: configurar");
               escribirTerceraLinea("");
               escribirCuartaLinea("Abajo: Atras");
               break;

            case CADA_SET_HORA:
               ui_dibujar_cada_edit(cada_h, cada_m, TRUE);
               break;

            case CADA_SET_MIN:
               ui_dibujar_cada_edit(cada_h, cada_m, FALSE);
               break;

            case CADA_CONFIRMAR:
               ui_dibujar_cada_confirmar(cada_h, cada_m);
               break;

            case OPCION_ELEGIR_PORCION:
               ui_dibujar_opcion_porc(porcion_g);
               break;

            case ELEGIR_GRAMOS_PORCION:
               ui_dibujar_edit_porc(porcion_g);
               break;

            case CONFIRMAR_GRAMOS_PORCION:
               ui_dibujar_confirm_porc(porcion_g);
               break;

            default:
               escribirPrimeraLinea("Estado invalido");
               escribirSegundaLinea("Volviendo...");
               escribirTerceraLinea("");
               escribirCuartaLinea("");
               break;
         }

         actualizarPantalla();
         estado_prev = estado;
      }

      // -------- FSM --------
      switch(estado) {

         case INICIO:
            if(inactivo()) { estado = PANTALLA_HORA; break; }
            if(ev_ok) estado = MENU;
            break;

         case PANTALLA_HORA:
            if((tickRead() - g_ultimoRefreshHora) >= CLOCK_REFRESH_MS) {
               char hora[16];
               obtenerHoraActual(hora);

               limpiarPantalla();
               escribirPrimeraLinea("Hora actual");
               escribirSegundaLinea(hora);
               escribirTerceraLinea("OK: menu");
               escribirCuartaLinea("Arr/Ab: inicio");
               actualizarPantalla();

               g_ultimoRefreshHora = tickRead();
            }

            if(ev_ok) estado = MENU;
            else if(ev_up || ev_down) estado = INICIO;
            break;

         case MENU: {
            if(inactivo()) { estado = PANTALLA_HORA; break; }

            static tick_t t_hora = 0;
            if((tickRead() - t_hora) >= 1000) {
               menu_dibujar(menu_sel);
               t_hora = tickRead();
            }

            if(ev_up) {
               menu_sel = (menu_sel == 0) ? (ITEM_COUNT - 1) : (menu_sel - 1);
            } else if(ev_down) {
               menu_sel = (menu_sel + 1) % ITEM_COUNT;
            }

            if(menu_sel != menu_sel_prev) {
               menu_dibujar(menu_sel);
               menu_sel_prev = menu_sel;
            }

            if(ev_ok) {
               switch(menu_sel) {
                  case ITEM_DISPENSAR:   estado = OPCION_DISPENSAR;   break;
                  case ITEM_DISP_EN_X:   estado = OPCION_DISP_EN_X;   break;
                  case ITEM_DISP_CADA_X: estado = OPCION_DISP_CADA_X; break;
                  case ITEM_PORCION:     estado = OPCION_ELEGIR_PORCION; break;
                  default: break;
               }
            }
            break;
         }

         case OPCION_DISPENSAR:
            if(ev_ok) {
               limpiarPantalla();
               escribirPrimeraLinea("");
               escribirSegundaLinea("Dispensando");
               escribirTerceraLinea("Alimento");
               escribirCuartaLinea("");
               actualizarPantalla();

               dispensarAlimento();
               delay(5000);
               estado = MENU;
            } else if(ev_down) {
               estado = MENU;
            }
            break;

         case OPCION_DISP_EN_X:
            if(ev_ok) {
               // ? Entrás a editar SIN tocar el programado:
               // - Si ya había un programado, editás ese.
               // - Si no, arrancás desde la hora actual.
               if(disp_prog_activo) {
                  disp_edit_h = disp_prog_hora;
                  disp_edit_m = disp_prog_min;
               } else {
                  rtc_get_hm(&disp_edit_h, &disp_edit_m);
               }

               estado = DISP_ENX_SET_HORA;
            } else if(ev_down) {
               estado = MENU;
            }
            break;

         case DISP_ENX_SET_HORA:
            if(ev_up) {
               disp_edit_h = (disp_edit_h + 1) % 24;
               ui_dibujar_set_hora(disp_edit_h, disp_edit_m, TRUE);
            } else if(ev_down) {
               disp_edit_h = (disp_edit_h == 0) ? 23 : (disp_edit_h - 1);
               ui_dibujar_set_hora(disp_edit_h, disp_edit_m, TRUE);
            }

            if(ev_ok) estado = DISP_ENX_SET_MIN;
            break;

         case DISP_ENX_SET_MIN:
            if(ev_up) {
               disp_edit_m = (disp_edit_m + 1) % 60;
               ui_dibujar_set_hora(disp_edit_h, disp_edit_m, FALSE);
            } else if(ev_down) {
               disp_edit_m = (disp_edit_m == 0) ? 59 : (disp_edit_m - 1);
               ui_dibujar_set_hora(disp_edit_h, disp_edit_m, FALSE);
            }

            if(ev_ok) estado = DISP_ENX_CONFIRMAR;
            break;

         case DISP_ENX_CONFIRMAR:
            if(ev_ok) {
               // ? Recién acá se guarda el horario programado real
               disp_prog_hora = disp_edit_h;
               disp_prog_min  = disp_edit_m;
               disp_prog_activo = TRUE;
               disp_prog_ejecutado = FALSE;

               // ? Evitar disparo instantáneo si lo programás "para ahora"
               rtc_t rtc;
               rtcRead(&rtc);
               if(rtc.hour == disp_prog_hora && rtc.min == disp_prog_min) {
                  disp_prog_ejecutado = TRUE; // se libera cuando cambie el minuto
               }

               limpiarPantalla();
               escribirPrimeraLinea("Programado!");
               escribirSegundaLinea("Esperando hora...");
               escribirTerceraLinea("");
               escribirCuartaLinea("");
               actualizarPantalla();

               delay(700);
               estado = MENU;
            } else if(ev_up) {
               estado = DISP_ENX_SET_HORA;
            } else if(ev_down) {
               estado = MENU;
            }
            break;

         case OPCION_DISP_CADA_X:
            if(ev_ok) {
               estado = CADA_SET_HORA;
            } else if(ev_down) {
               estado = MENU;
            }
            break;

         case CADA_SET_HORA:
            if(ev_up) {
               if(cada_h < 23) cada_h++;
               ui_dibujar_cada_edit(cada_h, cada_m, TRUE);
            } else if(ev_down) {
               if(cada_h > 0) cada_h--;
               ui_dibujar_cada_edit(cada_h, cada_m, TRUE);
            }

            if(ev_ok) estado = CADA_SET_MIN;
            break;

         case CADA_SET_MIN:
            if(ev_up) {
               if(cada_m < 59) cada_m++;
               ui_dibujar_cada_edit(cada_h, cada_m, FALSE);
            } else if(ev_down) {
               if(cada_m > 0) cada_m--;
               ui_dibujar_cada_edit(cada_h, cada_m, FALSE);
            }

            if(ev_ok) estado = CADA_CONFIRMAR;
            break;

         case CADA_CONFIRMAR:
            if(ev_ok) {
               cada_set_intervalo(cada_h, cada_m);

               limpiarPantalla();
               escribirPrimeraLinea("Guardado!");
               if(cada_activo) {
                  char buf[12];
                  sprintf(buf, "%02d:%02d", cada_h, cada_m);
                  escribirSegundaLinea(buf);
               } else {
                  escribirSegundaLinea("OFF");
               }
               escribirTerceraLinea("");
               escribirCuartaLinea("");
               actualizarPantalla();

               delay(700);
               estado = MENU;
            } else if(ev_up) {
               estado = CADA_SET_HORA;
            } else if(ev_down) {
               estado = MENU;
            }
            break;

         case OPCION_ELEGIR_PORCION:
            if(ev_ok) {
               estado = ELEGIR_GRAMOS_PORCION;
            } else if(ev_down) {
               estado = MENU;
            }
            break;

         case ELEGIR_GRAMOS_PORCION:
            if(ev_up) {
               if(porcion_g + PORCION_STEP_G <= PORCION_MAX_G) porcion_g += PORCION_STEP_G;
               ui_dibujar_edit_porc(porcion_g);
            } else if(ev_down) {
               if(porcion_g >= (PORCION_MIN_G + PORCION_STEP_G)) porcion_g -= PORCION_STEP_G;
               ui_dibujar_edit_porc(porcion_g);
            }

            if(ev_ok) {
               estado = CONFIRMAR_GRAMOS_PORCION;
            }
            break;

         case CONFIRMAR_GRAMOS_PORCION:
            if(ev_ok) {
               limpiarPantalla();
               escribirPrimeraLinea("Guardado!");
               {
                  char buf[24];
                  sprintf(buf, "%u g", (unsigned)porcion_g);
                  escribirSegundaLinea(buf);
               }
               escribirTerceraLinea("");
               escribirCuartaLinea("");
               actualizarPantalla();
               delay(700);

               estado = MENU;
            } else if(ev_down) {
               estado = MENU; // cancelar
            } else if(ev_up) {
               estado = ELEGIR_GRAMOS_PORCION;
            }
            break;

         default:
            estado = INICIO;
            break;
      }
   }
}