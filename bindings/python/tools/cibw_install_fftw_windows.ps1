# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..\..")
$FftwDir = Join-Path $RepoRoot "deps\fftw3"
$ZipPath = Join-Path $FftwDir "fftw.zip"

New-Item -ItemType Directory -Force -Path $FftwDir | Out-Null

if (-not (Test-Path (Join-Path $FftwDir "libfftw3-3.lib"))) {
    Invoke-WebRequest "https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip" -OutFile $ZipPath
    Expand-Archive -Force $ZipPath -DestinationPath $FftwDir
    Remove-Item $ZipPath

    $LibExe = Get-Command lib.exe -ErrorAction SilentlyContinue
    if (-not $LibExe) {
        throw "lib.exe is not available; initialize an x64 MSVC developer environment first"
    }

    foreach ($Name in @("libfftw3-3", "libfftw3f-3", "libfftw3l-3")) {
        & $LibExe.Source /machine:x64 "/def:$FftwDir\$Name.def" "/out:$FftwDir\$Name.lib"
        if ($LASTEXITCODE -ne 0) {
            throw "lib.exe failed to create $Name.lib"
        }
    }
}

Write-Host "FFTW staged in $FftwDir"
