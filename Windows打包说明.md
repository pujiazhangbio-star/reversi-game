# Windows Packaging Guide (ZIP Auto-Build)

This project includes two ready-to-use packaging scripts that build a distributable ZIP for Windows users. The ZIP contains `reversi.exe`, required SFML DLLs, and `fonts/DejaVuSans.ttf` so text renders consistently.

## Prerequisites
- Make sure `fonts/DejaVuSans.ttf` exists (already included in `fonts/`).
- Build `reversi.exe` first (Release preferred).

## Option A: MSYS2 UCRT64 (MinGW)
1. Install MSYS2 and open the “MSYS2 UCRT64” shell.
2. Install SFML and toolchain (once):
   - pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-sfml
3. Configure and build:
   - cmake -B build -S . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
   - cmake --build build --config Release
4. From a regular Windows CMD/PowerShell (or in Explorer), run:
   - scripts\pack-msys2-ucrt64.bat

Output: `Reversi-win64.zip` at project root.

## Option B: Visual Studio + vcpkg
1. Install vcpkg and integrate with VS.
2. Acquire SFML via vcpkg (x64-windows):
   - vcpkg install sfml:x64-windows
3. Configure CMake with vcpkg toolchain and build Release to `build/`.
4. Run PowerShell script:
   - pwsh -File scripts/pack-vs-vcpkg.ps1 -BuildDir build -VcpkgBinDir vcpkg_installed/x64-windows/bin

Output: `Reversi-win64.zip` at project root.

## What’s inside the ZIP
- reversi.exe
- SFML runtime DLLs (auto-detected 2.x or 3.x naming)
- common dependency DLLs (freetype, zlib, etc. when present)
- fonts/DejaVuSans.ttf
- run.bat (double-click to launch)

## Sending to users
Send the single `Reversi-win64.zip` file. Users can unzip anywhere and double‑click `run.bat` (or `reversi.exe`). No installation required.

## Troubleshooting
- If text is invisible: ensure `fonts/DejaVuSans.ttf` is present in the ZIP.
- If the app doesn’t start on user’s PC, a missing extra DLL might be the cause. Re-run the packaging script on your Windows machine; it tries to include common dependencies automatically.
- If you used a different build folder, pass `-BuildDir` to the script accordingly.
