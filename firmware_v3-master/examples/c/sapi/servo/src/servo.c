#include "servo.h"

// Pin del motor/servo
#define SERVO_PIN GPIO8

// Duty fijo ? velocidad constante
// Ajustá estos valores si gira muy lento / rápido
#define SERVO_PULSE_US  1500   // 1.5 ms (rotación continua típica)
#define SERVO_PERIOD_US 20000  // 20 ms

void girarMotor(uint32_t msTiempo)
{
   // Asegurarse de que el pin sea salida
   gpioConfig(SERVO_PIN, GPIO_OUTPUT);

   uint32_t elapsed = 0;

   while(elapsed < msTiempo) {
      gpioWrite(SERVO_PIN, ON);
      delayInaccurateUs(SERVO_PULSE_US);
      gpioWrite(SERVO_PIN, OFF);
      delayInaccurateUs(SERVO_PERIOD_US - SERVO_PULSE_US);

      elapsed += 20; // cada ciclo ? 20 ms
   }
}
