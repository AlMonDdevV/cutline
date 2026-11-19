<#
.SYNOPSIS
    Configure, build, test or run CUTLINE on Windows inside a Visual Studio developer shell.
.EXAMPLE
    scripts\build.ps1 -Target configure
    scripts\build.ps1 -Target build
    scripts\build.ps1 -Target test
    scripts\build.ps1 -Target run
    scripts\build.ps1 -Target screenshot --screenshot screens\main.png --size 1600x900
#>
[CmdletBinding()]
param(
    [ValidateSet('configure', 'build', 'test', 'run', 'screenshot')]
    [string]$Target = 'build',
    [string]$Preset = 'windows-msvc',
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Rest = @()
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

$qtVersion = (Get-Content (Join-Path $root 'tooling\qt-version.txt') -Raw).Trim()
if (-not $env:QT_ROOT_DIR) { $env:QT_ROOT_DIR = "C:\Qt\$qtVersion\msvc2022_64" }
if (-not (Test-Path $env:QT_ROOT_DIR)) {
    throw "Qt $qtVersion was not found at $env:QT_ROOT_DIR. See docs/building.md."
}
if (-not $env:VCPKG_ROOT) { $env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable('VCPKG_ROOT', 'User') }
if (-not $env:VCPKG_ROOT) { throw 'VCPKG_ROOT is not set. See docs/building.md.' }

if (-not $env:VSCMD_VER) {
    $installer = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer'
    $env:PATH = "$installer;$env:PATH"   # VsDevCmd calls vswhere by name
    $vswhere = Join-Path $installer 'vswhere.exe'
    $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vs) { throw 'Visual Studio with the C++ tools was not found. See docs/building.md.' }
    Import-Module (Join-Path $vs 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
    Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
}

$exe = Join-Path $root "build\$Preset\cutline.exe"
Push-Location $root
try {
    switch ($Target) {
        'configure' { cmake --preset $Preset @Rest }
        'build'     { cmake --build --preset $Preset @Rest }
        'test'      { ctest --preset $Preset @Rest }
        { $_ -in 'run', 'screenshot' } {
            $env:PATH = "$env:QT_ROOT_DIR\bin;$env:PATH"
            if ($Target -eq 'run') {
                if ($Rest.Count -gt 0) { Start-Process -FilePath $exe -ArgumentList $Rest }
                else { Start-Process -FilePath $exe }
                $global:LASTEXITCODE = 0
            } else {
                & $exe @Rest
            }
        }
    }
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} finally {
    Pop-Location
}
