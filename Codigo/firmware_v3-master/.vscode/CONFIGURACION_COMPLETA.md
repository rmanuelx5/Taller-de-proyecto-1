# ✅ Configuración Completa de VS Code para EDU-CIAA-NXP

## 📁 Archivos Creados/Configurados

Se han creado los siguientes archivos en la carpeta `.vscode/`:

### 1️⃣ **tasks.json** - Tareas de Compilación y Programación
Contiene 8 tareas predefinidas:
- ✅ **Build (Compilar)** - `Ctrl+Shift+B` - Compila el proyecto
- 🧹 **Clean (Limpiar)** - Limpia archivos compilados del programa actual
- 🧹 **Clean All** - Limpia todos los programas
- 📥 **Download to Flash** - Descarga el programa a la placa
- 🗑️ **Erase Flash** - Borra la memoria flash
- 🎯 **Select Program** - Selecciona el programa a compilar
- 🎯 **Select Board** - Selecciona la placa objetivo
- ➕ **New Program** - Crea un nuevo programa desde plantilla

### 2️⃣ **c_cpp_properties.json** - Configuración de IntelliSense
Configura el analizador de código C/C++ con:
- ✅ Todas las rutas de includes (sapi, lpc_open, freertos, etc.)
- ✅ Defines del preprocesador (CHIP_LPC43XX, ARM_MATH_CM4, etc.)
- ✅ Compilador ARM GCC con flags correctos
- ✅ Estándares C99 y C++11
- ✅ Uso del archivo compile_commands.json para mejor análisis

### 3️⃣ **launch.json** - Configuración de Debugging
Dos configuraciones de debugging:
- 🐛 **Debug EDU-CIAA-NXP** - Compila, descarga y debuggea (F5)
- 🔗 **Attach EDU-CIAA-NXP** - Se conecta a una sesión existente

⚠️ **Nota**: Requiere la extensión **Cortex-Debug**

### 4️⃣ **settings.json** - Configuración del Workspace
Configuraciones específicas del proyecto:
- 📍 Variables de configuración (`educiaa.programPath`, `educiaa.programName`)
- 🚫 Exclusión de archivos compilados en búsquedas
- ⚙️ Configuración de C/C++ IntelliSense
- 🔧 Rutas de herramientas (ARM toolchain, OpenOCD, GDB)
- 📝 Formato del editor (tabs, espacios)

### 5️⃣ **extensions.json** - Extensiones Recomendadas
Lista de extensiones que VS Code sugerirá instalar:
- **C/C++** (ms-vscode.cpptools)
- **Cortex-Debug** (marus25.cortex-debug)
- **ARM** (dan-c-underwood.arm)
- **Makefile Tools** (ms-vscode.makefile-tools)
- **Hex Editor** (ms-vscode.hexeditor)

### 6️⃣ **sapi.code-snippets** - Snippets de Código
Atajos rápidos para programación con SAPI:
- `sapi_main` - Template básico
- `sapi_blinky` - Ejemplo de LED parpadeante
- `sapi_gpio_write` - Escribir GPIO
- `sapi_uart_write` - Enviar por UART
- `freertos_task` - Crear tarea de FreeRTOS
- Y muchos más...

### 7️⃣ **README_VSCODE.md** - Documentación Completa
Guía detallada con:
- 📋 Requisitos previos
- 🚀 Configuración inicial
- 🔨 Instrucciones de compilación
- 📥 Descarga a la placa
- 🐛 Debugging
- 🆘 Solución de problemas

### 8️⃣ **check_tools.ps1** - Script de Verificación
Script de PowerShell que verifica:
- ✓ Instalación de ARM GCC
- ✓ Instalación de Make
- ✓ Instalación de OpenOCD
- ✓ Instalación de GDB (opcional)
- ✓ Archivos de configuración de VS Code

### 9️⃣ **firmware_v3.code-workspace** - Workspace File
Archivo de workspace para abrir el proyecto completo.

---

## 🚀 Primeros Pasos

### 1. Instalar Herramientas

Ejecuta el script de verificación:
```powershell
cd .vscode
.\check_tools.ps1
```

Si faltan herramientas, instálalas según las instrucciones del script.

### 2. Instalar Extensiones

Al abrir el proyecto en VS Code, aparecerá una notificación para instalar las extensiones recomendadas. Haz clic en **"Install All"**.

O instálalas manualmente:
1. `Ctrl+Shift+X` para abrir extensiones
2. Busca e instala las extensiones listadas en `.vscode/extensions.json`

### 3. Configurar Programa

**Opción A - Editar archivos:**
```makefile
# board.mk
BOARD = edu_ciaa_nxp

# program.mk
PROGRAM_PATH = examples/c/sapi/gpio
PROGRAM_NAME = blinky
```

**Opción B - Usar tareas:**
- `Ctrl+Shift+P` → "Tasks: Run Task" → "Select Program"

### 4. Compilar

Presiona `Ctrl+Shift+B` o ejecuta:
```powershell
make all
```

### 5. Descargar a la Placa

1. Conecta la EDU-CIAA-NXP
2. `Ctrl+Shift+P` → "Tasks: Run Task" → "Download to Flash"

O desde terminal:
```powershell
make download
```

### 6. Debugging (Opcional)

1. Conecta la placa
2. Presiona `F5`
3. El programa se compilará, descargará y se detendrá en `main()`

---

## 🎯 Características Principales

### ✨ IntelliSense Completo
- Autocompletado de funciones SAPI
- Navegación a definiciones (`F12`)
- Ver todas las referencias (`Shift+F12`)
- Información al pasar el mouse

### 🔨 Compilación Integrada
- Compilar con `Ctrl+Shift+B`
- Errores mostrados en la vista de "Problemas"
- Click en errores para ir directamente al código

### 📥 Programación de Placa
- Descarga directa desde VS Code
- No necesitas cambiar a Embedded IDE
- Todo desde un solo entorno

### 🐛 Debugging Avanzado
- Breakpoints visuales
- Step into/over/out
- Visualización de variables y registros
- Peripheral viewer (con SVD file)

### ⚡ Snippets Rápidos
- Escribe `sapi_` y presiona `Ctrl+Space`
- Templates completos con `sapi_main`
- Ejemplos listos para usar

---

## 📊 Comparación: Embedded IDE vs VS Code

| Característica | Embedded IDE | VS Code |
|---|---|---|
| IntelliSense | ⚠️ Básico | ✅ Avanzado |
| Autocompletado | ⚠️ Limitado | ✅ Completo |
| Debugging | ✅ Sí | ✅ Sí (con extensión) |
| Compilación | ✅ Integrada | ✅ Integrada |
| Descarga a placa | ✅ Sí | ✅ Sí |
| Navegación código | ⚠️ Básica | ✅ Avanzada |
| Snippets | ❌ No | ✅ Personalizables |
| Git integrado | ❌ No | ✅ Sí |
| Terminal integrada | ❌ No | ✅ Sí |
| Extensiones | ❌ No | ✅ Miles |
| Multiplataforma | ❌ Solo Windows | ✅ Win/Mac/Linux |

---

## 🔧 Personalización

### Cambiar Programa Activo

Edita `.vscode/settings.json`:
```json
"educiaa.programPath": "examples/c/sapi/gpio",
"educiaa.programName": "blinky"
```

### Cambiar Rutas de Herramientas

Si tus herramientas están en otras ubicaciones:
```json
"cortex-debug.armToolchainPath": "C:/TuRuta/gcc-arm/bin",
"cortex-debug.openocdPath": "C:/TuRuta/openocd/bin/openocd.exe"
```

### Agregar Más Tareas

Edita `.vscode/tasks.json` y agrega nuevas tareas basadas en comandos make.

---

## 🆘 Problemas Comunes

### "arm-none-eabi-gcc no encontrado"
- Instala GNU ARM Embedded Toolchain
- Asegúrate de agregarlo al PATH del sistema
- Reinicia VS Code

### "make no encontrado"
- Instala GNU Make para Windows
- Agrega al PATH
- Reinicia VS Code

### IntelliSense no funciona
- Verifica que la extensión C/C++ esté instalada
- `Ctrl+Shift+P` → "C/C++: Reset IntelliSense Database"
- Reinicia VS Code

### Debugging no funciona
- Verifica que Cortex-Debug esté instalado
- Verifica que OpenOCD esté en el PATH
- Revisa la configuración en `settings.json`
- Los errores en launch.json son normales hasta que instales Cortex-Debug

---

## 📚 Recursos

- **Documentación**: `.vscode/README_VSCODE.md`
- **Firmware v3**: https://github.com/ciaa/firmware_v3
- **SAPI Docs**: `libs/sapi/README.md`
- **Ejemplos**: Carpeta `examples/`

---

## ✅ Checklist de Configuración

- [ ] Herramientas instaladas (arm-gcc, make, openocd)
- [ ] Extensiones de VS Code instaladas
- [ ] Script check_tools.ps1 ejecutado sin errores
- [ ] Archivos board.mk y program.mk configurados
- [ ] Primer programa compilado exitosamente
- [ ] Programa descargado a la placa
- [ ] Debugging probado (opcional)

---

**¡Listo para trabajar con tu EDU-CIAA-NXP en VS Code!** 🎉

Para cualquier duda, consulta `README_VSCODE.md` o la documentación oficial del proyecto.
