# STM32F429BIT6 demo - flash firmware via OpenOCD (no build)
#
# Usage:
#   .\flash.ps1                  # Flash Debug build
#   .\flash.ps1 -Config Release  # Flash Release build
#
# Run build.ps1 first to generate the .elf file.

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

$ProjectRoot = $PSScriptRoot
$OpenOcd = "C:\Program Files\xpack-openocd-0.12.0-7\bin\openocd.exe"
$OpenOcdCfg = Join-Path $ProjectRoot "openocd.cfg"
$ElfPath = Join-Path $ProjectRoot "build\$Config\stm32f429bit6_demo.elf"

if (-not (Test-Path $OpenOcd)) {
    Write-Error "OpenOCD not found: $OpenOcd"
}

if (-not (Test-Path $OpenOcdCfg)) {
    Write-Error "OpenOCD config not found: $OpenOcdCfg"
}

if (-not (Test-Path $ElfPath)) {
    Write-Host ""
    Write-Host "[Flash 失败] 未找到固件, 请先编译" -ForegroundColor Red
    Write-Host "  缺少文件: $ElfPath" -ForegroundColor Yellow
    Write-Host "  请先执行: .\build.ps1" -ForegroundColor Yellow
    Write-Host "  或按:     Ctrl+Shift+B (Build Debug)" -ForegroundColor Yellow
    Write-Host ""
    exit 1
}

$elfForOpenOcd = $ElfPath -replace '\\', '/'
$programCmd = "program $elfForOpenOcd verify reset exit"

Write-Host "Flashing ($Config) ..."
Write-Host "  $ElfPath"
& $OpenOcd -f $OpenOcdCfg -c $programCmd
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Flash done." -ForegroundColor Green