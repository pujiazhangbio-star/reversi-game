param(
  [string]$BuildDir = "build",
  [string]$VcpkgBinDir = "vcpkg_installed/x64-windows/bin",
  [string]$ZipName = "Reversi-win64.zip"
)

$ErrorActionPreference = 'Stop'
Write-Host "=== Packaging Reversi (vcpkg) ==="

if (!(Test-Path "$BuildDir/reversi.exe")) {
  Write-Error "[Error] $BuildDir/reversi.exe not found. Build first (cmake --build $BuildDir --config Release)."
}

$dist = "dist"
if (Test-Path $dist) { Remove-Item -Recurse -Force $dist }
New-Item -ItemType Directory -Path $dist | Out-Null

Copy-Item "$BuildDir/reversi.exe" "$dist/" -Force

# Detect SFML major version (DLL naming 2 vs 3)
$sfmlVer = 2
if (Test-Path (Join-Path $VcpkgBinDir 'sfml-graphics-3.dll')) { $sfmlVer = 3 }

if ($sfmlVer -eq 3) {
  $sfmlDlls = @('sfml-graphics-3.dll','sfml-window-3.dll','sfml-system-3.dll')
} else {
  $sfmlDlls = @('sfml-graphics-2.dll','sfml-window-2.dll','sfml-system-2.dll')
}

foreach ($dll in $sfmlDlls) {
  $path = Join-Path $VcpkgBinDir $dll
  if (Test-Path $path) { Copy-Item $path "$dist/" -Force } else { Write-Warning "Missing $dll in $VcpkgBinDir" }
}

# Extra common deps if present (freetype, zlib, etc.)
$extra = @('freetype.dll','zlib1.dll')
foreach ($e in $extra) {
  $ep = Join-Path $VcpkgBinDir $e
  if (Test-Path $ep) { Copy-Item $ep "$dist/" -Force }
}

if (Test-Path "fonts/DejaVuSans.ttf") {
  Copy-Item "fonts" "$dist/" -Recurse -Force
} else {
  Write-Warning "fonts/DejaVuSans.ttf missing. Text may not render."
}

# Create run.bat for end user convenience
$runBat = @(
  '@echo off',
  'echo Launching Reversi...',
  'start "" "%~dp0reversi.exe"'
)
$runBat | Set-Content -Path (Join-Path $dist 'run.bat') -Encoding ASCII

# Zip it
if (Test-Path $ZipName) { Remove-Item $ZipName -Force }
Compress-Archive -Path "$dist/*" -DestinationPath $ZipName -Force

if (Test-Path $ZipName) {
  Write-Host "[OK] Created $ZipName (SFML $sfmlVer). Distribute this zip."
} else {
  Write-Error "Failed to create $ZipName"
}

Write-Host "Done."
