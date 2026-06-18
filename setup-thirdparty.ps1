# Fetch third-party sources not stored in this repo.
param(
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$ThirdPartyRoot = Join-Path $ProjectRoot "ThirdParty"
$FlashDbRoot = Join-Path $ThirdPartyRoot "FlashDB"
$FlashDbMarker = Join-Path $FlashDbRoot "inc\fdb_def.h"
$LwIpRoot = Join-Path $ThirdPartyRoot "LwIP"
$LwIpMarker = Join-Path $LwIpRoot "src\include\lwip\opt.h"
$J1939Marker = Join-Path $ThirdPartyRoot "Open-SAE-J1939\Src\Open_SAE_J1939\Open_SAE_J1939.h"
$IsotpMarker = Join-Path $ThirdPartyRoot "isotp-c\isotp.c"
$CubeLwIpRoot = Join-Path $env:USERPROFILE "STM32Cube\Repository\STM32Cube_FW_F4_V1.28.3\Middlewares\Third_Party\LwIP"

function Sync-LwIp {
    param([switch]$ForceCopy)

    if ((Test-Path $LwIpMarker) -and -not $ForceCopy) {
        Write-Host "LwIP already present: $LwIpRoot" -ForegroundColor DarkGray
        return
    }

    if (-not (Test-Path (Join-Path $CubeLwIpRoot "src\include\lwip\opt.h"))) {
        Write-Error "STM32Cube LwIP not found at $CubeLwIpRoot. Install STM32CubeF4 V1.28.3 or copy LwIP into ThirdParty\LwIP manually."
    }

    if ($ForceCopy -and (Test-Path $LwIpRoot)) {
        Write-Host "Removing existing LwIP ..."
        Remove-Item -Recurse -Force $LwIpRoot
    }

    Write-Host "Copying LwIP from STM32Cube package ..."
    New-Item -ItemType Directory -Force -Path $LwIpRoot | Out-Null
    Copy-Item -Recurse -Force (Join-Path $CubeLwIpRoot "src") (Join-Path $LwIpRoot "src")
    Copy-Item -Recurse -Force (Join-Path $CubeLwIpRoot "system") (Join-Path $LwIpRoot "system")
    Write-Host "LwIP ready." -ForegroundColor Green
}

function Sync-CanThirdParty {
    param([switch]$ForceCopy)

    if ((Test-Path $J1939Marker) -and (Test-Path $IsotpMarker) -and -not $ForceCopy) {
        Write-Host "CAN third-party packages already present." -ForegroundColor DarkGray
        return
    }

    if (-not (Test-Path $J1939Marker) -or $ForceCopy) {
        if ($ForceCopy -and (Test-Path (Join-Path $ThirdPartyRoot "Open-SAE-J1939"))) {
            Remove-Item -Recurse -Force (Join-Path $ThirdPartyRoot "Open-SAE-J1939")
        }
        Write-Host "Cloning Open-SAE-J1939 (shallow) ..."
        & git clone --depth 1 https://github.com/DanielMartensson/Open-SAE-J1939.git (Join-Path $ThirdPartyRoot "Open-SAE-J1939")
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to clone Open-SAE-J1939."
        }
    }

    if (-not (Test-Path $IsotpMarker) -or $ForceCopy) {
        if ($ForceCopy -and (Test-Path (Join-Path $ThirdPartyRoot "isotp-c"))) {
            Remove-Item -Recurse -Force (Join-Path $ThirdPartyRoot "isotp-c")
        }
        Write-Host "Cloning isotp-c (shallow) ..."
        & git clone --depth 1 https://github.com/SimonCahill/isotp-c.git (Join-Path $ThirdPartyRoot "isotp-c")
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to clone isotp-c."
        }
    }

    Write-Host "CAN third-party packages ready." -ForegroundColor Green
}

if ((Test-Path $FlashDbMarker) -and (Test-Path $LwIpMarker) -and (Test-Path $J1939Marker) -and (Test-Path $IsotpMarker) -and -not $Force) {
    Write-Host "Third-party packages already present." -ForegroundColor DarkGray
    exit 0
}

if (-not (Test-Path $FlashDbMarker)) {
    if ($Force -and (Test-Path $FlashDbRoot)) {
        Write-Host "Removing existing FlashDB ..."
        Remove-Item -Recurse -Force $FlashDbRoot
    }

    Write-Host "Cloning FlashDB (shallow) ..."
    & git clone --depth 1 https://github.com/armink/FlashDB.git $FlashDbRoot
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to clone FlashDB. Check network or clone manually into ThirdParty\FlashDB."
    }

    Write-Host "FlashDB ready." -ForegroundColor Green
}

Sync-LwIp -ForceCopy:$Force
Sync-CanThirdParty -ForceCopy:$Force
