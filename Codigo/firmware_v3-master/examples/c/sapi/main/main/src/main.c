/*==================[inclusions]=============================================*/

#include "sapi.h"
#include "pantalla.h"
#include "esp8266_at.h"
#include "loadcell.h"
#include "ui.h"        // <- declara loop_ui()
#include "servo.h"     // si el main configura el pin o querés usar algo del servo

/*==================[macros and definitions]=================================*/

#define BAUD_RATE 115200

// --- CONFIGURACION DE LA RED WI-FI CREADA ---
#define WIFI_SSID "test"
#define WIFI_PASS "0123456789"
#define WIFI_CHAN 5
#define WIFI_ENCR 3
#define delay_ms 10000

// --- HW PINS ---
#define SERVO_PIN      GPIO8

#define BOTON_ACEPTAR  GPIO4
#define BOTON_ABAJO    GPIO2
#define BOTON_ARRIBA   GPIO0

/*==================[main]===================================================*/

int main(void)
{
   
   boardConfig();
   tickConfig(1);
   rtcConfig();

   delay(3000); // Esperar que todo se estabilice

   // --- Botones (pull-up) ---
   gpioConfig(BOTON_ACEPTAR, GPIO_INPUT_PULLUP);
   gpioConfig(BOTON_ABAJO,   GPIO_INPUT_PULLUP);
   gpioConfig(BOTON_ARRIBA,  GPIO_INPUT_PULLUP);

   // --- Servo ---
   gpioConfig(SERVO_PIN, GPIO_OUTPUT);

   // --- ESP8266 ---
   esp_cfg_t cfg = {
      .uart_esp = UART_232,
      .uart_dbg = UART_USB,
      .debug_echo = TRUE,
      .usb_bridge = TRUE
   };

   esp_setup_uarts(&cfg);

   if(!esp_init_ap_server(&cfg, WIFI_SSID, WIFI_PASS, WIFI_CHAN, WIFI_ENCR, delay_ms)) {
      // si querés: mostrar error en pantalla o loguear por UART_USB
      // mostrar_toast("ESP ERROR", "", 1500);
   }

   // --- Pantalla ---
   pantalla_init();
   mostrar_toast("Bienvenido!", "", 1200);

   // --- Balanza ---
   hx711_init();
   delay(1000); // estabilizar

   /*int32_t tara_raw = ObtenerTaraRawUnaVez();
   if (tara_raw == 0x7FFFFFFF) {
      mostrar_toast("TARA ERROR", "HX711 timeout", 1500);
   } else {
      char line1[22];
      char line2[22];

      // Pantalla: 2 líneas (ajustá el formato a tu driver)
      snprintf(line1, sizeof(line1), "TARA RAW:");
      snprintf(line2, sizeof(line2), "%ld", (long)tara_raw);

      mostrar_toast(line1, line2, 5000);

      // UART (para copiar facil)
      char msg[80];
      sprintf(msg, "TARA_RAW0 = %ld\r\n", (long)tara_raw);
      uartWriteString(UART_USB, msg);
   }*/
   
   // --- MEF principal (while(1)) ---
   loop_ui();

   return 0;
}