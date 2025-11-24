# Reversi (黑白棋) — 构建说明

项目包含两个可执行目标：

- `reversi` — 基于 SFML 的 GUI 版本（依赖 SFML）
- `reversi_console` — 纯控制台版本（无 SFML 依赖）

CMake 提供两个开关：
- `BUILD_GUI` (默认 ON) — 是否构建 GUI 目标（需要 SFML）
- `BUILD_CONSOLE` (默认 ON) — 是否构建控制台目标

如果在没有 SFML 的环境下生成构建系统，CMake 会跳过 GUI 目标，但仍会生成控制台目标（如果启用）。

## macOS (推荐使用 Homebrew)

1. 安装工具与依赖：

```bash
# 安装 Homebrew（如果尚未安装）
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

brew update
brew install cmake sfml
```

2. 构建（在项目根目录运行）：

```bash
# 生成构建系统（包含控制台与 GUI）
cmake -S . -B build -DBUILD_CONSOLE=ON -DBUILD_GUI=ON

# 构建控制台版本
cmake --build build --config Release --target reversi_console -j$(sysctl -n hw.ncpu)

# 构建 GUI 版本（如果 SFML 可用）
cmake --build build --config Release --target reversi -j$(sysctl -n hw.ncpu)
```

3. 运行：

```bash
# 控制台
./build/reversi_console

# GUI
./build/reversi
```

> 直接用 `g++` 编译控制台也可以（不需要 CMake）：
>
> g++ -std=c++17 -Wall -Wextra -g3 console_othello.cpp -o output/reversi_console
>
> 若直接用 `g++` 链接 GUI 版本，请确保指定 Homebrew 的 include 与 lib 路径，并链接 `-lsfml-graphics -lsfml-window -lsfml-system`，但推荐使用 CMake 来处理平台差异。

## Windows（推荐使用 vcpkg）

1. 安装 vcpkg 并安装 SFML（示例使用 x64-windows 静态或动态 triplet）：

```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install sfml[graphics,window,system]:x64-windows
```

2. 使用 CMake（指定 vcpkg 工具链）：

```powershell
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -DBUILD_CONSOLE=ON -DBUILD_GUI=ON
cmake --build build --config Release --target reversi
```

3. CI / 打包：

- 本仓库已包含用于 Windows 的 GitHub Actions workflow（`.github/workflows/windows-single-exe.yml`），用于在 Windows runner 上使用 vcpkg 构建并上传 artifact。若需要我可以继续协助调整或触发 workflow。

## 常见问题

- 构建时报错找不到 `SFML/Graphics.hpp`：表示你的系统没有安装 SFML，或 CMake 没找到 SFML。请按上面步骤在 macOS 上安装 Homebrew 的 `sfml`，或在 Windows 上通过 vcpkg 安装。

- macOS 上运行 GUI 程序出现缺少 Framework 错误：确保使用 CMake 并且 SFML 是通过 Homebrew 安装，这样 CMake 能找到正确的库和 rpath 设置。

- 我只想本地测试控制台：可以直接编译 `console_othello.cpp`（见上面 `g++` 示例）。


如果你同意，我会：
- 把这些变更提交到仓库（已在工作区直接修改文件）。
- 接着帮你在本机（macOS）用你安装好的工具继续构建；如果你还没有安装，我会提供一步一步的命令并等待你授权运行。