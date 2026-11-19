<#
.SYNOPSIS
    Builds the portable Windows download: dist\CUTLINE-<version>-windows-x64.zip (+ .sha256).
.DESCRIPTION
    Run after scripts\build.ps1 -Target build. Copies cutline.exe and the FFmpeg DLLs it uses, runs windeployqt for
    Qt, QML and the Visual C++ runtime, adds licences and notices, zips it, then proves the zip is self-contained by
    running a screenshot from the extracted copy with Qt removed from PATH.
.EXAMPLE
    scripts\package-windows.ps1 -Version 0.1.0-preview
#>
[CmdletBinding()]
param(
    [string]$Version = '0.1.0-preview',
    [string]$Preset = 'windows-msvc'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$qtVersion = (Get-Content (Join-Path $root 'tooling\qt-version.txt') -Raw).Trim()
$qtBin = if ($env:QT_ROOT_DIR) { Join-Path $env:QT_ROOT_DIR 'bin' } else { "C:\Qt\$qtVersion\msvc2022_64\bin" }
$build = Join-Path $root "build\$Preset"
$name = "CUTLINE-$Version-windows-x64"
$dist = Join-Path $root 'dist'
$stage = Join-Path $dist $name

if (-not $env:VSCMD_VER) {
    $installer = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer'
    $env:PATH = "$installer;$env:PATH"
    $vs = & (Join-Path $installer 'vswhere.exe') -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    Import-Module (Join-Path $vs 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
    Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
}

if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force (Join-Path $stage 'licenses') | Out-Null

Copy-Item (Join-Path $build 'cutline.exe') $stage
Get-ChildItem $build -Filter '*.dll' | Where-Object { $_.Name -match '^(avcodec|avformat|avutil|avfilter|swresample|swscale)-\d+\.dll$' } |
    Copy-Item -Destination $stage

& (Join-Path $qtBin 'windeployqt.exe') --release --no-translations --compiler-runtime `
    --qmldir (Join-Path $root 'src\app\qml') (Join-Path $stage 'cutline.exe') | Out-Null
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed ($LASTEXITCODE)" }

Copy-Item (Join-Path $root 'LICENSE') (Join-Path $stage 'LICENSE.txt')
Copy-Item (Join-Path $root 'packaging\windows\README.txt') $stage
Copy-Item (Join-Path $root 'packaging\windows\THIRD-PARTY-NOTICES.txt') $stage
Copy-Item (Join-Path $root 'resources\fonts\Inter-OFL.txt') (Join-Path $stage 'licenses')
Copy-Item (Join-Path $root 'resources\fonts\JetBrainsMono-OFL.txt') (Join-Path $stage 'licenses')
$ffmpegSrc = Get-ChildItem 'C:\dev\vcpkg\buildtrees\ffmpeg\src' -Directory | Select-Object -First 1
Copy-Item (Join-Path $ffmpegSrc.FullName 'COPYING.LGPLv2.1') (Join-Path $stage 'licenses\LGPL-2.1.txt')
Copy-Item (Join-Path $ffmpegSrc.FullName 'COPYING.LGPLv3') (Join-Path $stage 'licenses\LGPL-3.0.txt')

$zip = Join-Path $dist "$name.zip"
if (Test-Path $zip) { Remove-Item -Force $zip }
Compress-Archive -Path $stage -DestinationPath $zip -CompressionLevel Optimal
$hash = (Get-FileHash -Algorithm SHA256 $zip).Hash.ToLower()
"$hash  $name.zip" | Set-Content -Encoding ascii (Join-Path $dist "$name.zip.sha256")

# Prove the zip is self-contained: extract it fresh and run it with no Qt or vcpkg on PATH.
$check = Join-Path $env:TEMP "cutline-zipcheck-$([guid]::NewGuid().ToString('N'))"
Expand-Archive $zip $check
$savedPath = $env:PATH
$env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
try {
    $png = Join-Path $check 'check.png'
    $p = Start-Process -FilePath (Join-Path $check "$name\cutline.exe") -ArgumentList @('--screenshot', $png, '--size', '1280x800') -PassThru -Wait
    if ($p.ExitCode -ne 0 -or -not (Test-Path $png)) { throw "the extracted build didn't run (exit $($p.ExitCode))" }
} finally {
    $env:PATH = $savedPath
    Remove-Item -Recurse -Force $check
}

$sizeMb = [math]::Round((Get-Item $zip).Length / 1MB, 1)
"OK $zip ($sizeMb MB) sha256 $hash"
