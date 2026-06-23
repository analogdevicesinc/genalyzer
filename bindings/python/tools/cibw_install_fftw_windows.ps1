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

    $VcVarsCandidates = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    )
    $VcVars = $VcVarsCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $VcVars) {
        throw "Could not find Visual Studio vcvars64.bat"
    }

    cmd /c "`"$VcVars`" && cd /d `"$FftwDir`" && lib /def:libfftw3-3.def && lib /def:libfftw3f-3.def && lib /def:libfftw3l-3.def"
}

Write-Host "FFTW staged in $FftwDir"
