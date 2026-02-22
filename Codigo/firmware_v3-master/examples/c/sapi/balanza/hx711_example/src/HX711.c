/*
 * HX711.c - Versión Corregida y Estabilizada
 */

#include "HX711.h"





//#############################################################################################
/*void hx711_init(hx711_t *hx711, gpioMap_t clk_pin, gpioMap_t dat_pin){
    hx711->clk_pin = clk_pin;
    hx711->dat_pin = dat_pin;

    gpioInit(clk_pin, GPIO_OUTPUT);
    gpioInit(dat_pin, GPIO_INPUT);
    
    // Configuración por defecto
    hx711->Again = 1; // Ganancia 128 por defecto
    hx711->Bgain = 2;
    hx711->Ascale = 1.0;
    hx711->Bscale = 1.0;
}*/

//#############################################################################################
void set_scale(hx711_t *hx711, float Ascale, float Bscale){
    hx711->Ascale = Ascale;
    hx711->Bscale = Bscale;
}

//#############################################################################################
void set_gain(hx711_t *hx711, uint8_t Again, uint8_t Bgain){
    switch (Again) {
        case 128:       // channel A, gain factor 128
            hx711->Again = 1;
            break;
        case 64:        // channel A, gain factor 64
            hx711->Again = 3;
            break;
    }
    // Canal B tiene ganancia fija de 32, requiere 2 pulsos
    hx711->Bgain = 2; 
}

//#############################################################################################
void set_offset(hx711_t *hx711, long offset, uint8_t channel){
    if(channel == CHANNEL_A) hx711->Aoffset = offset;
    else hx711->Boffset = offset;
}

//############################################################################################
// Función optimizada para leer bits
/*uint8_t shiftIn(hx711_t *hx711) {
    uint8_t value = 0;
    for (uint8_t i = 0; i < 8; i++) {
        gpioWrite(hx711->clk_pin, ON);
        // Aumentamos un poco el retardo para dar estabilidad
        delayInaccurateUs(10); // Antes era 1
        
        value <<= 1;
        
        if (gpioRead(hx711->dat_pin)) {
            value |= 1;
        }
        
        gpioWrite(hx711->clk_pin, OFF);
        delayInaccurateUs(10); // Antes era 1
    }
    return value;
}*/

uint8_t shiftIn(hx711_t *hx711) {
    uint8_t value = 0;

    for (uint8_t i = 0; i < 8; i++) {

        // Pulso de clock
        gpioWrite(hx711->clk_pin, ON);
        delayInaccurateUs(10);

        gpioWrite(hx711->clk_pin, OFF);
        delayInaccurateUs(10);

        // Muestreo después del flanco de bajada
        value <<= 1;
        if (gpioRead(hx711->dat_pin)) {
            value |= 1;
        }
    }

    return value;
}


//############################################################################################
bool is_ready(hx711_t *hx711) {
    return (gpioRead(hx711->dat_pin) == 0);
}

//############################################################################################
void wait_ready(hx711_t *hx711) {
    while (!is_ready(hx711)) {
        delay(1);
    }
}

//############################################################################################
/*long read(hx711_t *hx711) {
    // DESACTIVAR INTERRUPCIONES: Crítico para que no falle la lectura
    noInterrupts();

    // Esperar a que el chip esté listo (si no lo esperamos afuera)
    // Nota: Si se bloquea aquí, revisar conexión de hardware.
    // wait_ready(hx711); <-- Mejor llamarlo fuera o asegurar timeout, pero dejémoslo simple.
    // Asumimos que wait_ready se llamó antes o el chip está rápido. 
    // Si tienes problemas de bloqueo, comenta el wait_ready interno.
    
    // Leemos 24 bits (3 bytes)
    uint32_t data2 = shiftIn(hx711);
    uint32_t data1 = shiftIn(hx711);
    uint32_t data0 = shiftIn(hx711);

    // Configurar ganancia para la PRÓXIMA lectura (Pulsos extra)
    // Usamos la variable Again configurada
    for (uint8_t i = 0; i < hx711->Again; i++) {
        gpioWrite(hx711->clk_pin, ON);
        delayInaccurateUs(10);
        gpioWrite(hx711->clk_pin, OFF);
        delayInaccurateUs(10);
    }

    // REACTIVAR INTERRUPCIONES
    interrupts();

    // Construir el valor de 24 bits
    uint32_t value = (data2 << 16) | (data1 << 8) | data0;

    // Extensión de signo (si el bit 23 es 1, el número es negativo)
    if (value & 0x800000) {
        value |= 0xFF000000;
    }

    return (long)value;
}*/

long read(hx711_t *hx711) {

    // esperar a que DOUT esté en 0 justo antes de clockear
    while(!is_ready(hx711)) { }   // sin delay (es corto)

    noInterrupts();

    uint32_t data2 = shiftIn(hx711);
    uint32_t data1 = shiftIn(hx711);
    uint32_t data0 = shiftIn(hx711);

    for (uint8_t i = 0; i < hx711->Again; i++) {
        gpioWrite(hx711->clk_pin, ON);
        delayInaccurateUs(10);
        gpioWrite(hx711->clk_pin, OFF);
        delayInaccurateUs(10);
    }

    interrupts();

    uint32_t value = (data2 << 16) | (data1 << 8) | data0;

    if (value & 0x800000) value |= 0xFF000000;

    return (long)value;
}



//############################################################################################
long read_average(hx711_t *hx711, int8_t times, uint8_t channel) {
    long sum = 0;
    // IMPORTANTE: Descartar la primera lectura que puede tener la ganancia vieja
    read(hx711); 
    
    for (int8_t i = 0; i < times; i++) {
        wait_ready(hx711);
        sum += read(hx711);
        // No necesitamos delay(1) aquí necesariamente, wait_ready se encarga
    }
    return sum / times;
}

//############################################################################################
double get_value(hx711_t *hx711, int8_t times, uint8_t channel) {
    long offset = (channel == CHANNEL_A) ? hx711->Aoffset : hx711->Boffset;
    return (double)(read_average(hx711, times, channel) - offset);
}

//############################################################################################
void tare(hx711_t *hx711, uint8_t times, uint8_t channel) {
    double sum = read_average(hx711, times, channel);
    set_offset(hx711, (long)sum, channel);
}

//############################################################################################
void tare_all(hx711_t *hx711, uint8_t times) {
    tare(hx711, times, CHANNEL_A);
    tare(hx711, times, CHANNEL_B); // Nota: Canal B no suele usarse, puedes comentarlo
}

//############################################################################################
float get_weight(hx711_t *hx711, int8_t times, uint8_t channel) {
    float scale = (channel == CHANNEL_A) ? hx711->Ascale : hx711->Bscale;
    if(scale == 0) scale = 1.0; // Evitar división por cero
    return (float)(get_value(hx711, times, channel) / scale);
}

