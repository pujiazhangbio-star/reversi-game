cmake_minimum_required(VERSION 3.16)
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

# Link depending on the found version
if(SFML_VERSION VERSION_GREATER_EQUAL 3)
  target_link_libraries(reversi PRIVATE SFML::Graphics SFML::Window SFML::System)
else()
  target_link_libraries(reversi PRIVATE sfml-graphics sfml-window sfml-system)
endif()

if(WIN32 AND STATIC_SINGLE_EXE)
  # Try to reduce runtime dependencies. Full static may still pull in some DLLs (e.g. for networking, freetype etc.).
  if(MINGW)
    target_link_options(reversi PRIVATE -static -static-libgcc -static-libstdc++)
  elseif(MSVC)
    # Use static runtime (/MT) – safe for a self-contained exe
    foreach(flag_var CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()
  endif()
  # SFML static libs expect SFML_STATIC macro when using legacy 2.x
  target_compile_definitions(reversi PRIVATE SFML_STATIC)
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
