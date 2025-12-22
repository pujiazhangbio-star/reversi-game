@echo off
REM 简单的 Windows 打包脚本（在 Windows 上使用 PowerShell / cmd）
REM 假设已安装 vcpkg 并将 toolchain 文件路径替换为实际路径
set VCPKG_TOOLCHAIN=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
mkdir build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN% -DBUILD_GUI=ON -DEMBED_FONT=ON -DSTATIC_SINGLE_EXE=ON
cmake --build build --config Release --target reversi -j

if exist build\Release\reversi.exe (
  echo Packaging...
  powershell -Command "Compress-Archive -Path build\Release\reversi.exe,fonts -DestinationPath reversi-windows.zip -Force"
  echo Packaged to reversi-windows.zip
) else (
  echo Build failed or reversi.exe not found in build\Release
)
