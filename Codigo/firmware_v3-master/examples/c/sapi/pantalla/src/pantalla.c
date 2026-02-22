#include "pantalla.h"

#include <string.h>
#include <stdio.h>

#include "sh1106.h"
#include "font5x7.h"

// Copiado tal cual de tu main (mismos defines)
#define OLED_X  0
#define LINE_H 16
#define OLED_W 128
#define CHAR_W 6



void pantalla_init(void)
{
   SH1106_Init();
   SH1106_Fill(0x00);
   SH1106_Flush();
}


// ---- Interna (en tu main era static inline) ----
static inline void escribirLinea(uint8_t linea, const char* mensaje)
{
   LCD_PutStr(OLED_X, linea * LINE_H, (char*)mensaje, &Font5x7);
}

// ---- API con los mismos nombres ----
void escribirPrimeraLinea(const char* mensaje) { escribirLinea(0, mensaje); }
void escribirSegundaLinea(const char* mensaje) { escribirLinea(1, mensaje); }
void escribirTerceraLinea(const char* mensaje) { escribirLinea(2, mensaje); }
void escribirCuartaLinea(const char* mensaje)  { escribirLinea(3, mensaje); }

void escribirALaDerecha(uint8_t linea, const char* texto)
{
   int len = (int)strlen(texto);
   int x = OLED_W - (len * CHAR_W);
   if(x < 0) x = 0;

   LCD_PutStr(x, linea * LINE_H, (char*)texto, &Font5x7);
}

void limpiarPantalla(void)
{
   SH1106_Fill(0x00);
}

void actualizarPantalla(void)
{
   SH1106_Flush();
}

// ---- Toasts (los dejé con mismo nombre que en tu main) ----
void mostrar_toast(const char* l1, const char* l2, uint32_t ms)
{
   limpiarPantalla();
   escribirPrimeraLinea(l1);
   escribirSegundaLinea(l2);
   escribirTerceraLinea("");
   escribirCuartaLinea("");
   actualizarPantalla();

   delay(ms);
}

void mostrar_toast_hora(const char* titulo, uint8_t h, uint8_t m, uint32_t ms)
{
   char hm[8];
   sprintf(hm, "%02d:%02d", h, m);

   limpiarPantalla();
   escribirPrimeraLinea(titulo);
   escribirSegundaLinea(hm);
   escribirTerceraLinea("");
   escribirCuartaLinea("");
   actualizarPantalla();

   delay(ms);
}
