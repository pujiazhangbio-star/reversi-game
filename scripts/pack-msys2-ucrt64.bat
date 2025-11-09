@echo off
setlocal
REM =============================================================
REM MSYS2 UCRT64 Packaging Script for Reversi (SFML 2 or 3)
REM Produces a zip you can send directly to Windows users.
REM Steps to use:
REM   1. Open MSYS2 UCRT64 shell
REM   2. cmake -B build -S . -G "MinGW Makefiles"
REM   3. cmake --build build --config Release
REM   4. Run: scripts\pack-msys2-ucrt64.bat
REM Output: dist\Reversi-win64.zip containing reversi.exe + DLLs + fonts
REM =============================================================

set "MSYS2_ROOT=C:\msys64"
set "BIN=%MSYS2_ROOT%\ucrt64\bin"
set "BUILD=build"
set "DIST=dist"
set "ZIPNAME=Reversi-win64.zip"

if not exist %BUILD%\reversi.exe (
  echo [Error] %BUILD%\reversi.exe not found. Build first.
  exit /b 1
)

if not exist fonts mkdir fonts >nul 2>&1
if not exist fonts\DejaVuSans.ttf (
  echo [Warn] fonts\DejaVuSans.ttf missing. UI text may not render. Place DejaVuSans.ttf under fonts\
)

if exist %DIST% rmdir /s /q %DIST% 2>nul
mkdir %DIST%

copy %BUILD%\reversi.exe %DIST%\ >nul

REM Detect SFML major version by presence of DLL names
set "SFML_VER=2"
if exist %BIN%\sfml-graphics-3.dll set "SFML_VER=3"

if %SFML_VER%==3 (
  copy %BIN%\sfml-graphics-3.dll %DIST%\ >nul
  copy %BIN%\sfml-window-3.dll %DIST%\ >nul
  copy %BIN%\sfml-system-3.dll %DIST%\ >nul
) else (
  copy %BIN%\sfml-graphics-2.dll %DIST%\ >nul
  copy %BIN%\sfml-window-2.dll %DIST%\ >nul
  copy %BIN%\sfml-system-2.dll %DIST%\ >nul
)

REM Common dependency DLLs (best-effort)
for %%D in (freetype-6.dll zlib1.dll libwinpthread-1.dll libstdc++-6.dll libgcc_s_seh-1.dll libbz2-1.dll libpng16-16.dll libbrotlicommon.dll libbrotlidec.dll libharfbuzz-0.dll libgraphite2.dll) do (
  if exist %BIN%\%%D copy %BIN%\%%D %DIST%\ >nul
)

REM Fonts
xcopy fonts %DIST%\fonts /E /I /Y >nul

REM Create run.bat inside dist for user convenience
(
  echo @echo off
  echo echo Launching Reversi...
  echo start "" "%~dp0reversi.exe"
) > %DIST%\run.bat

REM Compress folder to zip (PowerShell fallback if tar not available)
where tar >nul 2>&1
if %ERRORLEVEL%==0 (
  powershell -NoLogo -NoProfile -Command "Compress-Archive -Path '%DIST%\*' -DestinationPath '%ZIPNAME%' -Force" >nul 2>&1
) else (
  powershell -NoLogo -NoProfile -Command "Compress-Archive -Path '%DIST%\*' -DestinationPath '%ZIPNAME%' -Force" >nul 2>&1
)

if exist %ZIPNAME% (
  echo [OK] Packaged %ZIPNAME% (SFML %SFML_VER%). Send this zip to users.
) else (
  echo [Error] Failed to create %ZIPNAME%. You can manually zip the dist folder.
)

endlocal
