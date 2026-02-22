/*
 * HX711.h - Cabecera corregida
 */

#ifndef APPLICATION_CORE_HX711_H_
#define APPLICATION_CORE_HX711_H_

#include "sapi.h"

#define CHANNEL_A 0
#define CHANNEL_B 1

// Macros para manejo de interrupciones en EDU-CIAA
#define interrupts() __enable_irq()
#define noInterrupts() __disable_irq()

typedef struct {
  gpioMap_t      clk_pin;
  gpioMap_t      dat_pin;
  long           Aoffset;
  float          Ascale;
  uint8_t        Again;
  long           Boffset;
  float          Bscale;
  uint8_t        Bgain;
} hx711_t;

/* --- Funciones de Configuración --- */
//void hx711_init(hx711_t *hx711, gpioMap_t clk_pin, gpioMap_t dat_pin);
void set_scale(hx711_t *hx711, float Ascale, float Bscale);
void set_gain(hx711_t *hx711, uint8_t Again, uint8_t Bgain);
void set_offset(hx711_t *hx711, long offset, uint8_t channel);

/* --- Funciones de Lectura --- */
long read(hx711_t *hx711);
long read_average(hx711_t *hx711, int8_t times, uint8_t channel);
double get_value(hx711_t *hx711, int8_t times, uint8_t channel);
float get_weight(hx711_t *hx711, int8_t times, uint8_t channel);

/* --- Funciones de Tara y Utilidades --- */
void tare(hx711_t *hx711, uint8_t times, uint8_t channel);
void tare_all(hx711_t *hx711, uint8_t times);
bool is_ready(hx711_t *hx711);
void wait_ready(hx711_t *hx711);

// Funciones internas auxiliares (opcional exponerlas)
uint8_t shiftIn(hx711_t *hx711);

#endif /* APPLICATION_CORE_HX711_H_ */