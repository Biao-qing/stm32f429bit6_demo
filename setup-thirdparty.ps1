# Fetch third-party sources not stored in this repo.
param(
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$ThirdPartyRoot = Join-Path $ProjectRoot "ThirdParty"
$FlashDbRoot = Join-Path $ThirdPartyRoot "FlashDB"
$FlashDbMarker = Join-Path $FlashDbRoot "inc\fdb_def.h"
$J1939Marker = Join-Path $ThirdPartyRoot "Open-SAE-J1939\Src\Open_SAE_J1939\Open_SAE_J1939.h"
$IsotpMarker = Join-Path $ThirdPartyRoot "isotp-c\isotp.c"

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

if ((Test-Path $FlashDbMarker) -and (Test-Path $J1939Marker) -and (Test-Path $IsotpMarker) -and -not $Force) {
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

Sync-CanThirdParty -ForceCopy:$Force
