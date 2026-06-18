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
$FlashDbMarker = Join-Path $ProjectRoot "ThirdParty\FlashDB\inc\fdb_def.h"
$LwIpMarker = Join-Path $ProjectRoot "ThirdParty\LwIP\src\include\lwip\opt.h"
$J1939Marker = Join-Path $ProjectRoot "ThirdParty\Open-SAE-J1939\Src\Open_SAE_J1939\Open_SAE_J1939.h"
$IsotpMarker = Join-Path $ProjectRoot "ThirdParty\isotp-c\isotp.c"
if (-not (Test-Path $FlashDbMarker) -or -not (Test-Path $LwIpMarker) -or -not (Test-Path $J1939Marker) -or -not (Test-Path $IsotpMarker)) {
    Write-Host "Third-party packages missing. Running setup-thirdparty.ps1 ..."
    & (Join-Path $ProjectRoot "setup-thirdparty.ps1")
}
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

function Update-CompileCommands {
    param([string]$BuildDirectory)

    $Src = Join-Path $BuildDirectory "compile_commands.json"
    $Dst = Join-Path $ProjectRoot "compile_commands.json"
    if (Test-Path $Src) {
        Copy-Item -Force $Src $Dst
        Write-Host "Updated compile_commands.json for IDE indexing." -ForegroundColor DarkGray
    }
}

function Invoke-CMakeConfigure {
    Write-Host "Configuring ($Config, $Generator) ..."
    & cmake -B $BuildDir `
        -DCMAKE_BUILD_TYPE=$Config `
        "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile" `
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON `
        -G $Generator
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Update-CompileCommands $BuildDir
}

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
    Invoke-CMakeConfigure
} elseif (-not (Test-CommandExists "ninja")) {
    # Switching generator requires a clean configure.
    $cacheFile = Join-Path $BuildDir "CMakeCache.txt"
    if (Test-Path $cacheFile) {
        $cache = Get-Content $cacheFile -Raw
        if ($cache -match "CMAKE_GENERATOR:INTERNAL=Ninja" -and $Generator -eq "MinGW Makefiles") {
            Write-Host "Build cache uses Ninja but Ninja is unavailable. Reconfiguring ..."
            Remove-Item -Recurse -Force $BuildDir
            Invoke-CMakeConfigure
        }
    }
}

if (-not (Test-Path (Join-Path $BuildDir "compile_commands.json"))) {
    Invoke-CMakeConfigure
}

Write-Host "Building ($Config) ..."
& cmake --build $BuildDir
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Update-CompileCommands $BuildDir

$ElfPath = Join-Path $BuildDir "stm32f429bit6_demo.elf"
if (Test-Path $ElfPath) {
    Write-Host ""
    Write-Host "Build succeeded: $ElfPath" -ForegroundColor Green
    & arm-none-eabi-size $ElfPath
} else {
    Write-Error "Build finished but ELF not found: $ElfPath"
}
