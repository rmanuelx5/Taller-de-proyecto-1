# Compile options
VERBOSE=n
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

# Libraries
USE_LPCOPEN=y
USE_SAPI=y

# Incluir archivos del driver OLED SH1106 que está en otro directorio
INCLUDES += -Iexamples/c/sapi/i2c/oled_sh1106/inc
INCLUDES += -Iexamples/c/sapi/balanza/hx711_example/inc
SRC += examples/c/sapi/i2c/oled_sh1106/src/sh1106.c
SRC += examples/c/sapi/i2c/oled_sh1106/src/font5x7.c
SRC += examples/c/sapi/balanza/hx711_example/src/loadcell.c

