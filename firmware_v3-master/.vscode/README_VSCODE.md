# Configuración de VS Code para EDU-CIAA-NXP

Este proyecto ha sido configurado para trabajar con la placa **EDU-CIAA-NXP** (LPC4337) en Visual Studio Code.

## 📋 Requisitos Previos

### 1. Herramientas necesarias

- **Visual Studio Code** (instalado)
- **GNU ARM Embedded Toolchain** (arm-none-eabi-gcc)
  - Descargar desde: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
  - Asegurarse de que esté en el PATH del sistema
  
- **OpenOCD** (para debugging y descarga a placa)
  - Windows: https://github.com/xpack-dev-tools/openocd-xpack/releases
  - Agregar al PATH del sistema

- **Make** (GNU Make para Windows)
  - Puede instalarse con: `choco install make` (si tienes Chocolatey)
  - O descargar desde: http://gnuwin32.sourceforge.net/packages/make.htm

### 2. Extensiones de VS Code Recomendadas

Al abrir el proyecto, VS Code te sugerirá instalar estas extensiones (definidas en `.vscode/extensions.json`):

- **C/C++** (ms-vscode.cpptools) - IntelliSense y navegación de código
- **Cortex-Debug** (marus25.cortex-debug) - Debugging de ARM Cortex
- **ARM** (dan-c-underwood.arm) - Soporte para lenguaje ensamblador ARM
- **Makefile Tools** (ms-vscode.makefile-tools) - Soporte para Makefiles
- **Hex Editor** (ms-vscode.hexeditor) - Para visualizar archivos .bin/.hex

## 🚀 Configuración Inicial

### 1. Verificar instalación de herramientas

Abre una terminal en VS Code (Ctrl+Shift+`) y verifica:

```powershell
arm-none-eabi-gcc --version
make --version
openocd --version
```

Si algún comando no es reconocido, necesitas instalarlo y agregarlo al PATH.

### 2. Configurar el programa a compilar

Edita los archivos `board.mk` y `program.mk` en la raíz del proyecto:

**board.mk:**
```makefile
BOARD = edu_ciaa_nxp
```

**program.mk:**
```makefile
PROGRAM_PATH = examples/c/sapi/gpio
PROGRAM_NAME = blinky
```

O usa las tareas predefinidas:
- **Select Program**: Abre el menú de selección de programas
- **Select Board**: Abre el menú de selección de placas

### 3. Ajustar configuración en settings.json (opcional)

Edita `.vscode/settings.json` y modifica:

```json
"educiaa.programPath": "examples/c/sapi/gpio",
"educiaa.programName": "blinky",
```

Esto afecta las rutas de debugging.

## 🔨 Compilación

### Usando Tareas de VS Code

1. Presiona `Ctrl+Shift+B` (atajo de compilación por defecto)
2. O usa `Ctrl+Shift+P` → "Tasks: Run Task" → "Build (Compilar)"

### Usando la Terminal

```powershell
make all
```

### Otras tareas disponibles

- **Clean**: Limpia archivos de compilación del programa actual
  ```powershell
  make clean
  ```

- **Clean All**: Limpia todos los programas
  ```powershell
  make clean_all
  ```

## 📥 Descarga a la Placa

### Usando Tareas de VS Code

`Ctrl+Shift+P` → "Tasks: Run Task" → "Download to Flash (Descargar a Flash)"

### Usando la Terminal

```powershell
make download
```

Esto:
1. Compila el proyecto (si es necesario)
2. Usa OpenOCD para descargar el programa a la placa

### Borrar la Flash

```powershell
make erase
```

## 🐛 Debugging

### Configuración de Cortex-Debug

El archivo `.vscode/launch.json` tiene dos configuraciones:

1. **Debug EDU-CIAA-NXP (OpenOCD)**: Compila, descarga y debuggea
2. **Attach EDU-CIAA-NXP (OpenOCD)**: Se conecta a una sesión en ejecución

### Iniciar Debugging

1. Conecta la placa EDU-CIAA-NXP via USB
2. Presiona `F5` o ve a Run → Start Debugging
3. El debugger se detendrá en `main()`

### Ajustar rutas de herramientas

Si las herramientas están en ubicaciones diferentes, edita `.vscode/settings.json`:

```json
"cortex-debug.armToolchainPath": "C:/ruta/a/tu/arm-toolchain/bin",
"cortex-debug.openocdPath": "openocd",
"cortex-debug.gdbPath": "arm-none-eabi-gdb"
```

## 📝 IntelliSense y Autocompletado

El archivo `.vscode/c_cpp_properties.json` configura IntelliSense con:

- Todas las rutas de include necesarias
- Defines del preprocesador
- Configuración del compilador ARM
- Estándar C99 y C++11

El archivo `compile_commands.json` en la raíz también proporciona información de compilación.

## 🔧 Estructura del Proyecto

```
firmware_v3-master/
├── .vscode/
│   ├── c_cpp_properties.json  # Configuración de IntelliSense
│   ├── launch.json            # Configuración de debugging
│   ├── settings.json          # Configuración del workspace
│   ├── tasks.json             # Tareas de compilación
│   └── extensions.json        # Extensiones recomendadas
├── board.mk                   # Selección de placa
├── program.mk                 # Selección de programa
├── Makefile                   # Makefile principal
├── examples/                  # Programas de ejemplo
├── libs/                      # Bibliotecas
└── scripts/                   # Scripts auxiliares
```

## 📚 Crear un Nuevo Programa

### Opción 1: Usando la tarea

`Ctrl+Shift+P` → "Tasks: Run Task" → "New Program (Nuevo Programa)"

### Opción 2: Manualmente

```powershell
make new_program
```

Esto creará una plantilla de programa en la ubicación que especifiques.

## ⚙️ Configuración Avanzada

### Modificar opciones de compilación

Edita el archivo `config.mk` dentro de tu programa:

```makefile
# Opciones de compilación
OPT=g              # Nivel de optimización (g=debug, 0,1,2,3,s)
USE_NANO=y         # Usar newlib-nano
USE_FPU=y          # Usar unidad de punto flotante
SEMIHOST=n         # Semihosting (para printf vía debugger)

# Bibliotecas
USE_LPCOPEN=y
USE_SAPI=y
USE_FREERTOS=n
```

### Cambiar el programa sin editar archivos

Usa los scripts interactivos:

```powershell
make select_program
make select_board
```

## 🆘 Solución de Problemas

### Error: "arm-none-eabi-gcc: command not found"

Instala GNU ARM Embedded Toolchain y agrégalo al PATH.

### Error: "make: command not found"

Instala GNU Make para Windows.

### Error al descargar: "Can't find openocd"

Instala OpenOCD y asegúrate de que esté en el PATH.

### IntelliSense no funciona correctamente

1. Presiona `Ctrl+Shift+P`
2. Escribe "C/C++: Reset IntelliSense Database"
3. Reinicia VS Code

### Los errores no aparecen en la vista de Problemas

Asegúrate de que la extensión C/C++ esté instalada y activa.

## 📖 Recursos Adicionales

- **Documentación oficial CIAA**: https://github.com/ciaa/firmware_v3
- **Documentación SAPI**: En `libs/sapi/README.md`
- **Ejemplos**: Carpeta `examples/`
- **OpenOCD**: https://openocd.org/

## 💡 Consejos

1. Usa `Ctrl+Click` sobre una función para ir a su definición
2. Usa `F12` para ir a la definición
3. Usa `Shift+F12` para ver todas las referencias
4. Usa `Ctrl+Space` para autocompletado
5. Configura `educiaa.programPath` y `educiaa.programName` en settings.json para que el debugging apunte al programa correcto

---

**¡Listo para programar!** 🎉
