# Compile options

VERBOSE=n
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

# Libraries

USE_LPCOPEN=y
USE_SAPI=y
INCLUDES += -Iexamples/c/sapi/i2c/oled_sh1106/inc
SRC += examples/c/sapi/i2c/oled_sh1106/src/sh1106.c
SRC += examples/c/sapi/i2c/oled_sh1106/src/font5x7.c