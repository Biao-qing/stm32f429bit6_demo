# Fetch third-party sources not stored in this repo.
param(
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$FlashDbRoot = Join-Path $ProjectRoot "Vendor\FlashDB"
$Marker = Join-Path $FlashDbRoot "inc\fdb_def.h"

if ((Test-Path $Marker) -and -not $Force) {
    Write-Host "FlashDB already present: $FlashDbRoot" -ForegroundColor DarkGray
    exit 0
}

if ($Force -and (Test-Path $FlashDbRoot)) {
    Write-Host "Removing existing FlashDB ..."
    Remove-Item -Recurse -Force $FlashDbRoot
}

Write-Host "Cloning FlashDB (shallow) ..."
& git clone --depth 1 https://github.com/armink/FlashDB.git $FlashDbRoot
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to clone FlashDB. Check network or clone manually into Vendor\FlashDB."
}

Write-Host "FlashDB ready." -ForegroundColor Green
