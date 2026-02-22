# Compile options
VERBOSE=n
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

# Libraries
USE_LPCOPEN=y
USE_SAPI=y
USE_FREERTOS=n
FREERTOS_HEAP_TYPE=5
LOAD_INRAM=n

# --- HX711 external peripheral (include + source) ---
CFLAGS += -I$(CIAA_FIRMWARE)/libs/sapi/sapi_v0.6.2/external_peripherals/balanza/hx711/inc
SRC_FILES += $(CIAA_FIRMWARE)/libs/sapi/sapi_v0.6.2/external_peripherals/balanza/hx711/src/HX711.c
