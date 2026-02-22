#include "loadcell.h"

static int32_t tara1 = 0;  // Tara dispensador
static int32_t tara2 = 0;  // Tara plato
const float escala = 0.00064617f;
int32_t g_tara_raw_disp = 0;



int32_t ObtenerTaraRawUnaVez(void)
{
   int32_t v[10];

   for(int i=0;i<10;i++){
      int32_t r = hx711_read_raw();
      if(r == 0x7FFFFFFF) return 0x7FFFFFFF; // timeout
      v[i] = r;
      delay(20);
   }

   // ordenar
   for(int i=0;i<10;i++){
      for(int j=i+1;j<10;j++){
         if(v[j] < v[i]) { int32_t t=v[i]; v[i]=v[j]; v[j]=t; }
      }
   }

   // mediana (prom de las dos del medio)
   return (v[4] + v[5]) / 2;
}

void loadcellInit(void) {
    Ready4read(HX711_DATA_PIN1);
    //Ready4read(HX711_DATA_PIN2);
    Tarar(HX711_DATA_PIN1, &tara1);
    //Tarar(HX711_DATA_PIN2, &tara2);
}

void Ready4read(int pin) {
    gpioConfig(pin, GPIO_INPUT);
    gpioConfig(HX711_SCK_PIN, GPIO_OUTPUT);
    while (gpioRead(pin));
}

int32_t ReadRaw(int pin) {
    int32_t Count = 0;
    uint32_t i;

    gpioWrite(HX711_SCK_PIN, LOW);
    while (gpioRead(pin));

    for (i = 0; i < 24; i++) {
        gpioWrite(HX711_SCK_PIN, HIGH);
        Count = Count << 1;
        gpioWrite(HX711_SCK_PIN, LOW);
        if (gpioRead(pin)) {
            Count++;
        }
    }

    gpioWrite(HX711_SCK_PIN, HIGH);
    gpioWrite(HX711_SCK_PIN, LOW);

    if (Count & 0x800000) {
        Count |= 0xFF000000;
    }

    return Count;
}

void Tarar(int pin, int32_t* tara_val) {
    *tara_val = 0;
    int32_t acum = 0;
    for (uint32_t i = 0; i < 10; i++) {
        acum += ReadRaw(pin);
        delay(50);
    }
    *tara_val = acum / 10;
    
    char msg[50];
    sprintf(msg, "Valor de tara calculado: %d\n", *tara_val);
    uartWriteString(UART_USB, msg);
}

float ObtenerPeso(int pin, int32_t tara_val) {
    int32_t acum = 0;
    for (int i = 0; i < 5; i++) {
        acum += ReadRaw(pin);
        delay(10);
    }
    int32_t rawValue = acum / 5;

    char msg[120];
    sprintf(msg, "raw=%ld  tara=%ld  diff=%ld\r\n",
            (long)rawValue, (long)tara_val, (long)(rawValue - tara_val));
    uartWriteString(UART_USB, msg);

    float peso = (float)(rawValue - tara_val) * escala;
    return fabs(peso);
}

float ObtenerPesoDispensador(void) {
    return ObtenerPeso(HX711_DATA_PIN1, tara1);
}

//float ObtenerPesoPlato(void) {
//    return ObtenerPeso(HX711_DATA_PIN2, tara2);
//}





// Espera a que el HX711 esté listo (DOUT = LOW)
static bool hx711_wait_ready(uint32_t timeout_ms) {
    uint32_t t0 = tickRead();
    while (gpioRead(HX711_DATA_PIN1)) {
        if (tickRead() - t0 > timeout_ms) {
            return false;
        }
    }
    return true;
}

void hx711_init(void) {
    gpioConfig(HX711_DATA_PIN1, GPIO_INPUT);
    gpioConfig(HX711_SCK_PIN, GPIO_OUTPUT);
    gpioWrite(HX711_SCK_PIN, LOW);
}

// Lectura RAW 24 bits con extensión de signo
int32_t hx711_read_raw(void) {
    int32_t value = 0;

    if (!hx711_wait_ready(1000)) {
        return 0x7FFFFFFF; // marca de timeout
    }

    for (int i = 0; i < 24; i++) {
        gpioWrite(HX711_SCK_PIN, HIGH);
        value = value << 1;
        if (gpioRead(HX711_DATA_PIN1)) {
            value++;
        }
        gpioWrite(HX711_SCK_PIN, LOW);
    }

    // Pulso extra: canal A, ganancia 128
    gpioWrite(HX711_SCK_PIN, HIGH);
    gpioWrite(HX711_SCK_PIN, LOW);

    // Extensión de signo (24 ? 32 bits)
    if (value & 0x800000) {
        value |= 0xFF000000;
    }

    return value;
}


// Lee HX711 varias veces y devuelve mediana
static int32_t sort_and_median5_i32(int32_t a[5]) {
   for(int i=0;i<5;i++){
      for(int j=i+1;j<5;j++){
         if(a[j] < a[i]) { int32_t t=a[i]; a[i]=a[j]; a[j]=t; }
      }
   }
   return a[2];
}


static bool_t hx711_read_median_i32(int32_t *out) {
   int32_t v[MED_N];

   for(int i=0;i<MED_N;i++){
      int32_t r = hx711_read_raw();
      if(r == 0x7FFFFFFF) return FALSE; // timeout según tu driver
      v[i] = r;
      delay(5);
   }

   *out = sort_and_median5_i32(v);
   return TRUE;
}

// Devuelve peso en gramos (entero, sin floats en printf)
bool_t balanza_leer_gramos(int32_t *out_g) {
   int32_t raw;
   if(!hx711_read_median_i32(&raw)) return FALSE;

   int32_t d = raw - (int32_t)TARA_RAW0;

   float grams_f = (float)d * SCALE_G_PER_COUNT;
   if(grams_f < 0) grams_f = -grams_f;

   *out_g = (int32_t)(grams_f + 0.5f); // redondeo a gramo entero
   return TRUE;
}

int32_t ObtenerPesoDispensador_g(void)
{
   int32_t g;
   if(!balanza_leer_gramos(&g)) return -1; // error/timeout
   return g;
}

int32_t hx711_tarar_raw(void)
{
   int32_t raw;
   int64_t acum = 0;

   for(int i = 0; i < 15; i++){
      if(!hx711_read_median_i32(&raw)) return 0x7FFFFFFF;
      acum += raw;
      delay(20);
   }

   return (int32_t)(acum / 15);
}

