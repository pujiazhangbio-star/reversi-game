cmake_minimum_required(VERSION 3.16)

# 强制使用静态运行库（必须在 project() 之前）
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(reversi_sfml LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer SFML 3
find_package(SFML 3 COMPONENTS Graphics Window System QUIET)
if(NOT SFML_FOUND)
  message(STATUS "SFML 3 not found, trying SFML 2.x ...")
  find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
endif()

option(EMBED_FONT "Embed fonts/DejaVuSans.ttf into the Windows executable (no external font file needed)" ON)
option(STATIC_SINGLE_EXE "Attempt static link on Windows (MinGW / MSVC) to minimize external DLLs" OFF)

add_executable(reversi
  reversi_sfml.cpp
)

if(WIN32 AND EMBED_FONT)
  # Resource script embeds the TTF file as RCDATA so we can load from memory
  target_sources(reversi PRIVATE resources.rc)
  target_compile_definitions(reversi PRIVATE EMBED_FONT_RESOURCE)
endif()

# 使用静态 SFML 库时必须定义 SFML_STATIC
target_compile_definitions(reversi PRIVATE SFML_STATIC)

# Link depending on the found version
if(SFML_VERSION VERSION_GREATER_EQUAL 3)
  target_link_libraries(reversi PRIVATE SFML::Graphics SFML::Window SFML::System)
else()
  target_link_libraries(reversi PRIVATE sfml-graphics sfml-window sfml-system)
endif()

if(WIN32 AND STATIC_SINGLE_EXE)
  if(MINGW)
    target_link_options(reversi PRIVATE -static -static-libgcc -static-libstdc++)
  endif()
endif()

# On macOS and Linux, help runtime find SFML shared libs when not using a package manager
if(APPLE)
  # Use rpath to Homebrew default install path as a convenience
  set_target_properties(reversi PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"  # adjust if needed
  )
elseif(UNIX)
  # Typical system lib dirs already in search path; customize if needed
endif()
