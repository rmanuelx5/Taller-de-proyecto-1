#ifndef PANTALLA_H
#define PANTALLA_H

#include "sapi.h"
#include <stdint.h>

void pantalla_init(void);

void limpiarPantalla(void);
void actualizarPantalla(void);

void escribirPrimeraLinea(const char* mensaje);
void escribirSegundaLinea(const char* mensaje);
void escribirTerceraLinea(const char* mensaje);
void escribirCuartaLinea(const char* mensaje);

void escribirALaDerecha(uint8_t linea, const char* texto);

void mostrar_toast(const char* l1, const char* l2, uint32_t ms);
void mostrar_toast_hora(const char* titulo, uint8_t h, uint8_t m, uint32_t ms);

#endif
