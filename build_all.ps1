# build_all.ps1
# Generates the native libgenalyzer Visual Studio solution via CMake,
# builds it, then builds the managed C# Genalyzer.sln.
#
# Usage:
#   .\build_all.ps1 [-Configuration Release|Debug] [-Arch x64|x86]
#
param(
    [ValidateSet("Release","Debug")]
    [string]$Configuration = "Release",

    [ValidateSet("x64","x86")]
    [string]$Arch = "x64"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root     = $PSScriptRoot
$NativeBuildDir = Join-Path $Root "build\$($Configuration.ToLower())"

# -------------------------------------------------------------------------
# Step 1 – Generate the native Visual Studio solution with CMake
# -------------------------------------------------------------------------
Write-Host "`n=== Configuring native project (CMake) ===" -ForegroundColor Cyan

if (-not (Test-Path $NativeBuildDir)) {
    New-Item -ItemType Directory -Path $NativeBuildDir | Out-Null
}

$cmakeArgs = @(
    "-S", $Root,
    "-B", $NativeBuildDir,
    "-G", "Visual Studio 17 2022",
    "-A", $Arch,
    "-DCMAKE_BUILD_TYPE=$Configuration",
    "-DBUILD_SHARED_LIBS=ON",
    "-DBUILD_TESTS_EXAMPLES=OFF",
    "-DBUILD_DOC=OFF"
)

& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed."
    exit 1
}

# -------------------------------------------------------------------------
# Step 2 – Build the native libraries
# -------------------------------------------------------------------------
Write-Host "`n=== Building native libraries ===" -ForegroundColor Cyan

& cmake --build $NativeBuildDir --config $Configuration --target genalyzer
if ($LASTEXITCODE -ne 0) {
    Write-Error "Native build failed."
    exit 1
}

Write-Host "Native build succeeded.  Binaries are in: $NativeBuildDir" `
    -ForegroundColor Green

# -------------------------------------------------------------------------
# Step 3 – Build the C# managed bindings
# -------------------------------------------------------------------------
Write-Host "`n=== Building C# bindings ===" -ForegroundColor Cyan

$CSharpSln = Join-Path $Root "bindings\C#\Genalyzer.sln"

& dotnet build $CSharpSln -c $Configuration
if ($LASTEXITCODE -ne 0) {
    Write-Error "C# build failed."
    exit 1
}

Write-Host "`n=== All builds succeeded ===" -ForegroundColor Green
