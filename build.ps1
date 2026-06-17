# STM32F429BIT6 demo - one-click build script
#
# Usage:
#   .\build.ps1                  # Debug build (default)
#   .\build.ps1 -Config Release  # Release build
#   .\build.ps1 -Clean           # Remove build dir only (no compile)
#
# Or double-click build.bat, or run from CMD:
#   build.bat
#   build.bat -Config Release
#
# Output:
#   build\Debug\stm32f429bit6_demo.elf
#   build\Release\stm32f429bit6_demo.elf
#
# Notes:
#   - ARM toolchain path is set in $ToolchainBin below
#   - Uses Ninja if available, otherwise MinGW Makefiles
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$ProjectRoot = $PSScriptRoot
$ToolchainBin = "C:\Program Files\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin"
$BuildDir = Join-Path $ProjectRoot "build\$Config"
$ToolchainFile = Join-Path $ProjectRoot "cmake\gcc-arm-none-eabi.cmake"

function Test-CommandExists {
    param([string]$Name)
    return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

if (-not (Test-Path $ToolchainBin)) {
    Write-Error "ARM toolchain not found: $ToolchainBin`nPlease update ToolchainBin in build.ps1."
}

$env:PATH = "$ToolchainBin;$env:PATH"

if (-not (Test-CommandExists "arm-none-eabi-gcc")) {
    Write-Error "arm-none-eabi-gcc is not available after updating PATH."
}

if (-not (Test-CommandExists "cmake")) {
    Write-Error "cmake not found. Please install CMake and add it to PATH."
}

$Generator = if (Test-CommandExists "ninja") { "Ninja" } else { "MinGW Makefiles" }

if ($Clean) {
    if (Test-Path $BuildDir) {
        Write-Host "Removing $BuildDir ..."
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "Clean done." -ForegroundColor Green
    } else {
        Write-Host "Nothing to clean: $BuildDir"
    }
    exit 0
}

if (-not (Test-Path $BuildDir)) {
    Write-Host "Configuring ($Config, $Generator) ..."
    & cmake -B $BuildDir `
        -DCMAKE_BUILD_TYPE=$Config `
        "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile" `
        -G $Generator
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} elseif (-not (Test-CommandExists "ninja")) {
    # Switching generator requires a clean configure.
    $cacheFile = Join-Path $BuildDir "CMakeCache.txt"
    if (Test-Path $cacheFile) {
        $cache = Get-Content $cacheFile -Raw
        if ($cache -match "CMAKE_GENERATOR:INTERNAL=Ninja" -and $Generator -eq "MinGW Makefiles") {
            Write-Host "Build cache uses Ninja but Ninja is unavailable. Reconfiguring ..."
            Remove-Item -Recurse -Force $BuildDir
            & cmake -B $BuildDir `
                -DCMAKE_BUILD_TYPE=$Config `
                "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile" `
                -G $Generator
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        }
    }
}

Write-Host "Building ($Config) ..."
& cmake --build $BuildDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$ElfPath = Join-Path $BuildDir "stm32f429bit6_demo.elf"
if (Test-Path $ElfPath) {
    Write-Host ""
    Write-Host "Build succeeded: $ElfPath" -ForegroundColor Green
    & arm-none-eabi-size $ElfPath
} else {
    Write-Error "Build finished but ELF not found: $ElfPath"
}
