# Script to locate msvcp140.dll and update path in ISS file

param(
    [Parameter(Mandatory=$true)]
    [string]$IssFilePath
)

# Function to find msvcp140.dll
function Find-MSVCP140 {
    $possiblePaths = @(
        "${env:ProgramFiles}\Microsoft Visual Studio\*\*\VC\Redist\MSVC\*\x64\Microsoft.VC*.CRT\msvcp140.dll",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\*\*\VC\Redist\MSVC\*\x64\Microsoft.VC*.CRT\msvcp140.dll",
        "${env:WINDIR}\System32\msvcp140.dll",
        "${env:WINDIR}\SysWOW64\msvcp140.dll"
    )
    
    foreach ($path in $possiblePaths) {
        $found = Get-ChildItem -Path $path -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            return $found.FullName
        }
    }
    
    return $null
}

# Locate msvcp140.dll
$dllPath = Find-MSVCP140

if (-not $dllPath) {
    Write-Error "msvcp140.dll not found on the system"
    exit 1
}

Write-Host "Found msvcp140.dll at: $dllPath"

# Check if ISS file exists
if (-not (Test-Path $IssFilePath)) {
    Write-Error "ISS file not found: $IssFilePath"
    exit 1
}

# Read ISS file content
$issContent = Get-Content $IssFilePath

# Update the path in ISS file (assuming pattern like Source: "path\to\msvcp140.dll")
$updatedContent = $issContent -replace 'Source:\s*"[^"]*msvcp140\.dll"', "Source: `"$dllPath`""

# Write updated content back to file
Set-Content -Path $IssFilePath -Value $updatedContent

Write-Host "Updated ISS file with new msvcp140.dll path"