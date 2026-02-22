# Script de verificación de herramientas para EDU-CIAA-NXP en VS Code
# Ejecutar con: .\check_tools.ps1

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Verificación de Herramientas EDU-CIAA" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$allOk = $true

# Función para verificar comando
function Test-Command {
    param (
        [string]$Command,
        [string]$DisplayName,
        [string]$InstallInfo
    )
    
    Write-Host "Verificando $DisplayName... " -NoNewline
    
    try {
        $null = Get-Command $Command -ErrorAction Stop
        Write-Host "[OK]" -ForegroundColor Green
        
        # Mostrar versión
        $version = & $Command --version 2>&1 | Select-Object -First 1
        Write-Host "  Versión: $version" -ForegroundColor Gray
        return $true
    }
    catch {
        Write-Host "[NO ENCONTRADO]" -ForegroundColor Red
        Write-Host "  $InstallInfo" -ForegroundColor Yellow
        return $false
    }
}

# Verificar ARM GCC
if (-not (Test-Command -Command "arm-none-eabi-gcc" -DisplayName "ARM GCC Toolchain" `
    -InstallInfo "Instalar desde: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm")) {
    $allOk = $false
}

Write-Host ""

# Verificar Make
if (-not (Test-Command -Command "make" -DisplayName "GNU Make" `
    -InstallInfo "Instalar con: choco install make o desde http://gnuwin32.sourceforge.net/packages/make.htm")) {
    $allOk = $false
}

Write-Host ""

# Verificar OpenOCD
if (-not (Test-Command -Command "openocd" -DisplayName "OpenOCD" `
    -InstallInfo "Instalar desde: https://github.com/xpack-dev-tools/openocd-xpack/releases")) {
    $allOk = $false
}

Write-Host ""

# Verificar GDB (opcional pero recomendado)
Write-Host "Verificando ARM GDB (opcional)... " -NoNewline
try {
    $null = Get-Command "arm-none-eabi-gdb" -ErrorAction Stop
    Write-Host "[OK]" -ForegroundColor Green
    $version = & arm-none-eabi-gdb --version 2>&1 | Select-Object -First 1
    Write-Host "  Versión: $version" -ForegroundColor Gray
}
catch {
    Write-Host "[NO ENCONTRADO]" -ForegroundColor Yellow
    Write-Host "  GDB es necesario para debugging. Viene con ARM GCC Toolchain." -ForegroundColor Gray
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

if ($allOk) {
    Write-Host "✓ Todas las herramientas necesarias están instaladas" -ForegroundColor Green
    Write-Host ""
    Write-Host "Siguiente paso:" -ForegroundColor Cyan
    Write-Host "1. Edita board.mk y program.mk para seleccionar tu programa" -ForegroundColor White
    Write-Host "2. Ejecuta 'make all' para compilar" -ForegroundColor White
    Write-Host "3. Ejecuta 'make download' para descargar a la placa" -ForegroundColor White
} else {
    Write-Host "✗ Faltan herramientas necesarias" -ForegroundColor Red
    Write-Host "  Por favor instala las herramientas faltantes antes de continuar" -ForegroundColor Yellow
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Verificar archivos de configuración de VS Code
Write-Host "Verificando archivos de configuración de VS Code..." -ForegroundColor Cyan
$vscodeFiles = @(
    ".vscode\c_cpp_properties.json",
    ".vscode\launch.json",
    ".vscode\settings.json",
    ".vscode\tasks.json",
    ".vscode\extensions.json"
)

$configOk = $true
foreach ($file in $vscodeFiles) {
    if (Test-Path $file) {
        Write-Host "  $file " -NoNewline
        Write-Host "[OK]" -ForegroundColor Green
    } else {
        Write-Host "  $file " -NoNewline
        Write-Host "[FALTA]" -ForegroundColor Red
        $configOk = $false
    }
}

if ($configOk) {
    Write-Host "✓ Configuración de VS Code completa" -ForegroundColor Green
} else {
    Write-Host "✗ Faltan archivos de configuración" -ForegroundColor Red
}

Write-Host ""
Write-Host "Para más información, consulta: .vscode\README_VSCODE.md" -ForegroundColor Cyan
