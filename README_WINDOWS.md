Windows 打包与分发说明

目标：生成一个可以直接在 Windows 上运行的单文件/单目录分发包，使同事下载后能直接打开并玩游戏。

两种选择（优先顺序）：

1) 单 exe（优选）
   - 要求：在 Windows 上用 Visual Studio + vcpkg 或 MSYS2 构建，启用 EMBED_FONT=ON 并尽量使用静态链接（STATIC_SINGLE_EXE=ON），以减少对外部 DLL 的依赖。
   - 构建步骤（示例，Visual Studio + vcpkg）：
     1. 安装 Visual Studio（含 Desktop C++ 工作负载）、CMake 与 vcpkg。
     2. 安装 SFML：
        ```powershell
        .\vcpkg\vcpkg.exe install sfml[audio,window,graphics]:x64-windows
        .\vcpkg\vcpkg.exe integrate install
        ```
     3. 配置并编译：
        ```powershell
        cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_GUI=ON -DEMBED_FONT=ON -DSTATIC_SINGLE_EXE=ON
        cmake --build build --config Release --target reversi -j
        ```
     4. 可选：将生成的 `reversi.exe` 放入一个空文件夹，运行它确认一切正常。
   - 注意：即使静态链接也可能依赖系统 DLL（例如 DirectX、MSVCP 等）。如果仍然依赖 SFML 动态 DLL，可把这些 DLL 一并放在 exe 同目录。

2) 单目录分发（简单可靠）
   - 把以下文件/文件夹放在同一目录并压缩成 ZIP：
     - `reversi.exe`（或 `reversi` 可执行文件）
     - `fonts/DejaVuSans.ttf`（若未嵌入）
     - `sounds/`（若使用声音）
     - `README.md`（运行说明）
   - 同事解压后直接运行 exe 即可。

我在仓库中已做的工作
- 提供了 CMake 的 `EMBED_FONT` 选项（默认 ON），可在 Windows 下把字体作为资源嵌入 exe。
- 在 `CMakeLists.txt` 中增加了 `install()` 与 CPack 配置，便于 `cpack` 生成 ZIP 包。

我可以帮你做的事
- 在 Windows 环境下为你实际构建单个 exe（需要在 Windows 上运行构建脚本或我在你提供 Windows CI 上运行）。
- 如果你希望项目产生一个 ZIP 分发包，我可以在 macOS 上运行 `cpack`（前提是已构建并有可安装目标），或给出 `pack-windows.bat` 脚本以在 Windows 上自动打包。

发送给同事的最小文件清单（如果你生成了单 exe）
- `reversi.exe`（主可执行文件）
- 可选：`fonts/DejaVuSans.ttf`（仅当你没有启用 EMBED_FONT）
- 可选：`sounds/` 文件夹（如果你希望包含音效）
- `README.md`（简短运行说明）

如果你确认要我把仓库里不必要的文件移走/归档，请告诉我你希望保留哪些（例如保留 `src/`、`fonts/`、`sounds/`、`CMakeLists.txt` 和 `README.md`），我会把其余文件移动到 `archive_unused/` 目录，而不是直接删除，以防误删。
