# 🚀 Configuración de VS Code Lista

Este proyecto ha sido configurado para trabajar con **Visual Studio Code** como entorno de desarrollo para la placa **EDU-CIAA-NXP**.

## 📖 Documentación Completa

Toda la información detallada está en la carpeta `.vscode/`:

- **[CONFIGURACION_COMPLETA.md](.vscode/CONFIGURACION_COMPLETA.md)** - Resumen de todos los archivos creados
- **[README_VSCODE.md](.vscode/README_VSCODE.md)** - Guía completa de uso

## ⚡ Inicio Rápido

### 1. Verificar Herramientas

Ejecuta en PowerShell:
```powershell
cd .vscode
.\check_tools.ps1
```

Necesitas tener instalado:
- ARM GCC Toolchain
- GNU Make
- OpenOCD

### 2. Instalar Extensiones de VS Code

Cuando abras el proyecto, VS Code te sugerirá instalar extensiones recomendadas. Acepta instalarlas.

### 3. Configurar tu Programa

Edita estos archivos:

**board.mk:**
```makefile
BOARD = edu_ciaa_nxp
```

**program.mk:**
```makefile
PROGRAM_PATH = examples/c/sapi/gpio
PROGRAM_NAME = blinky
```

### 4. Compilar

Presiona `Ctrl+Shift+B`

### 5. Descargar a la Placa

`Ctrl+Shift+P` → "Tasks: Run Task" → "Download to Flash"

## 📚 Más Información

Lee la [documentación completa](.vscode/README_VSCODE.md) para:
- Configuración detallada
- Uso de debugging
- Snippets de código
- Solución de problemas

---

**¡Todo listo para programar en VS Code!** 🎉
