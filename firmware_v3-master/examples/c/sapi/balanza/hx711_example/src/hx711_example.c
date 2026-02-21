#include "sapi.h"
#include "loadcell.h"   // hx711_init(), hx711_read_raw()
#include <stdio.h>
#include <stdlib.h>

#define MED_N 5

// ====== TU CALIBRACION GUARDADA ======
#define TARA_RAW0           116876        // raw0 que obtuviste
#define SCALE_G_PER_COUNT   0.002502f     // g/count aprox (ajustable)
// =====================================

static int32_t sort_and_median5(int32_t a[5]) {
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (a[j] < a[i]) {
                int32_t t = a[i];
                a[i] = a[j];
                a[j] = t;
            }
        }
    }
    return a[2];
}

static bool hx711_read_median(int32_t *out) {
    int32_t v[MED_N];

    for (int i = 0; i < MED_N; i++) {
        int32_t r = hx711_read_raw();
        if (r == 0x7FFFFFFF) return false; // timeout (segun nuestro driver)
        v[i] = r;
        delay(5);
    }

    *out = sort_and_median5(v);
    return true;
}

int main(void) {
    boardConfig();
    uartConfig(UART_USB, 115200);

    hx711_init();

    char msg[160];
    uartWriteString(UART_USB, "\r\n=== HX711 PESO EN VIVO (SIN CALIBRAR) ===\r\n");

    sprintf(msg, "TARA_RAW0=%ld\r\nSCALE=%.6f g/count\r\n\r\n",
            (long)TARA_RAW0, (double)SCALE_G_PER_COUNT);
    uartWriteString(UART_USB, msg);

    while (1) {
        int32_t raw;
        if (!hx711_read_median(&raw)) {
            uartWriteString(UART_USB, "TIMEOUT\r\n");
            delay(200);
            continue;
        }

        int32_t d = raw - (int32_t)TARA_RAW0;

        // Convertir a gramos (positivo)
        float grams = (float)d * SCALE_G_PER_COUNT;
        if (grams < 0) grams = -grams;

        // imprimir sin %f: 1 decimal
        int g10 = (int)(grams * 10.0f + 0.5f);
        int g_int = g10 / 10;
        int g_dec = g10 % 10;

        // También mostramos raw y delta por debug
        sprintf(msg, "raw=%ld d=%ld peso=%d.%d g\r\n",
                (long)raw, (long)d, g_int, g_dec);
        uartWriteString(UART_USB, msg);

        delay(200);
    }
}

