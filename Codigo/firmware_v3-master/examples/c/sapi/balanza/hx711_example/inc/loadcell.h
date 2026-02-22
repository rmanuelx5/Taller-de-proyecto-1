#ifndef LOADCELL_H
#define LOADCELL_H

#include "sapi.h"
#include <math.h>  // Agregado para fabs()

#define HX711_SCK_PIN   GPIO5
#define HX711_DATA_PIN1 GPIO7  // Celda dispensador
#define SCALE_G_PER_COUNT   0.001510f   // g/count (tu calibración)    
#define MED_N 5
#define TARA_RAW0  -30648  



int32_t hx711_read_raw(void);
void hx711_init(void);

bool_t balanza_leer_gramos(int32_t *out_g);   // <- la vamos a exportar
int32_t ObtenerPesoDispensador_g(void);

int32_t ObtenerTaraRawUnaVez(void);

void loadcellInit(void);
void Ready4read(int pin);
float ObtenerPesoDispensador(void);
float ObtenerPesoPlato(void);
void Tarar(int pin, int32_t* tara_val);
static int32_t sort_and_median5_i32(int32_t a[5]);
static bool_t hx711_read_median_i32(int32_t *out);

void hx711_init(void);
int32_t hx711_read_raw(void);

#endif