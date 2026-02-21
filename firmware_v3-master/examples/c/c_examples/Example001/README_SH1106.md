# Ejemplo de pantalla OLED SH1106 via I2C

Este ejemplo demuestra cómo usar una pantalla OLED SH1106 de 128x64 píxeles conectada via I2C.

## Hardware requerido

- EDU-CIAA-NXP (o compatible)
- Pantalla OLED SH1106 128x64 con interfaz I2C
- Cables de conexión

## Conexiones

### Para EDU-CIAA-NXP:

| Pantalla | EDU-CIAA-NXP | Descripción |
|----------|--------------|-------------|
| VCC      | 3.3V o 5V    | Alimentación (verificar voltaje del módulo) |
| GND      | GND          | Tierra |
| SCL      | T1 (I2C0_SCL)| Reloj I2C |
| SDA      | T2 (I2C0_SDA)| Datos I2C |

## Dirección I2C

La dirección I2C por defecto es **0x3C**. Algunos módulos pueden usar **0x3D**.

Si tu pantalla usa 0x3D, edita el archivo `inc/sh1106_i2c.h` y cambia:

```c
#define SH1106_I2C_ADDR       0x3C
```

a:

```c
#define SH1106_I2C_ADDR       0x3D
```

## Funcionalidad

El programa realiza lo siguiente:

1. Inicializa el puerto I2C y la pantalla SH1106
2. Dibuja un borde rectangular alrededor de la pantalla
3. Dibuja una cruz en el centro
4. Dibuja un círculo en el centro
5. En un bucle infinito:
   - Parpadea el LED azul cada 500ms
   - Alterna entre pantalla completamente blanca y negra con figuras

Si la inicialización falla (pantalla no conectada o dirección incorrecta), el LED rojo parpadeará rápidamente.

## Compilación

Desde la raíz del proyecto:

```bash
make all
```

O usar la tarea de VSCode: **Build (Compilar)**

## Descarga a la placa

```bash
make download
```

O usar la tarea de VSCode: **Download to Flash (Descargar a Flash)**

## API del driver

### Inicialización
```c
bool_t SH1106_Init(i2cMap_t i2cPort);
```

### Funciones de control
```c
void SH1106_Contrast(uint8_t contrast);        // 0-255
void SH1106_SetInvert(uint8_t inv_state);      // LCD_INVERT_ON/OFF
void SH1106_SetDisplayState(uint8_t state);    // LCD_ON/OFF
void SH1106_Orientation(uint8_t orientation);  // LCD_ORIENT_NORMAL/CW/CCW/180
void SH1106_Fill(uint8_t pattern);             // Llenar buffer (0x00=negro, 0xFF=blanco)
void SH1106_Flush(void);                       // Enviar buffer a pantalla
```

### Funciones de dibujo
```c
void LCD_Pixel(uint8_t X, uint8_t Y, uint8_t Mode);
void LCD_HLine(uint8_t X1, uint8_t X2, uint8_t Y);
void LCD_VLine(uint8_t X, uint8_t Y1, uint8_t Y2);
void LCD_Rect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2);
void LCD_FillRect(uint8_t X1, uint8_t Y1, uint8_t X2, uint8_t Y2);
void LCD_Line(int16_t X1, int16_t Y1, int16_t X2, int16_t Y2);
void LCD_Circle(int16_t X, int16_t Y, uint8_t R);
```

### Modos de píxel
Antes de dibujar, puedes establecer el modo:
```c
LCD_PixelMode = LCD_PSET;  // Encender píxel (por defecto)
LCD_PixelMode = LCD_PRES;  // Apagar píxel
LCD_PixelMode = LCD_PINV;  // Invertir píxel
```

## Troubleshooting

### LED rojo parpadeando rápidamente
- La pantalla no está conectada
- Dirección I2C incorrecta (prueba cambiar de 0x3C a 0x3D)
- Conexiones SDA/SCL invertidas
- Problema de alimentación en la pantalla

### Pantalla en blanco
- Verificar que se llame `SH1106_Flush()` después de dibujar
- Verificar contraste con `SH1106_Contrast(128)`
- Probar diferente orientación

### Dibujo distorsionado
- Verificar que las coordenadas estén dentro de 0-127 (X) y 0-63 (Y)
- Probar diferentes orientaciones

## Archivos del proyecto

- `src/example001.c` - Programa principal de ejemplo
- `src/sh1106_i2c.c` - Implementación del driver I2C
- `inc/sh1106_i2c.h` - Declaraciones del driver I2C
- `README_SH1106.md` - Este archivo

## Notas

- El driver usa un buffer de video RAM (vRAM) de 1KB
- Todas las operaciones de dibujo se realizan en el buffer
- Llamar a `SH1106_Flush()` para actualizar la pantalla física
- La velocidad I2C está configurada a 100kHz para mayor compatibilidad
