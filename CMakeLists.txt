# Top-level CMake build for Xconq.
#
# Xconq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

cmake_minimum_required(VERSION 3.20)

project(Xconq VERSION 7.5.0 LANGUAGES C CXX)

set(XCONQ_VERSION_RELEASE "0pre")

include(GNUInstallDirs)
include(CheckIncludeFileCXX)
include(CheckTypeSize)

# ---------------------------------------------------------------------------
# User-facing options.
# ---------------------------------------------------------------------------

option(XCONQ_UI_CURSES "Build the curses interface (cconq)" ON)
option(XCONQ_UI_SDL    "Build the SDL 2 interface (sdlconq)" ON)

set(XCONQ_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/xconq" CACHE PATH
    "Installed location of the games library, images, and UI scripts")
set(XCONQ_SCORES_DIR "/var/lib/xconq/scores" CACHE PATH
    "Directory used for score files at run time")

# ---------------------------------------------------------------------------
# Dependency discovery.  Each UI is skipped (with a warning) when its
# dependencies are missing, so a bare kernel build always works.
# ---------------------------------------------------------------------------

if(XCONQ_UI_CURSES)
    find_package(Curses)
    if(NOT CURSES_FOUND)
        message(WARNING "Curses not found; skipping the cconq interface")
    endif()
endif()

if(XCONQ_UI_SDL)
    find_package(SDL2)
    if(NOT SDL2_FOUND)
        message(WARNING "SDL2 not found (try libsdl2-dev); skipping the SDL interface")
    endif()
    find_package(X11)
endif()

# ---------------------------------------------------------------------------
# Platform introspection feeding the generated acdefs.h.
# ---------------------------------------------------------------------------

check_include_file_cxx(dirent.h HAVE_DIRENT_H)

set(CMAKE_EXTRA_INCLUDE_FILES)
check_type_size(int SIZEOF_INT LANGUAGE CXX)
check_type_size(long SIZEOF_LONG LANGUAGE CXX)
check_type_size("long long" SIZEOF_LONG_LONG LANGUAGE CXX)

# Which curses flavor did we find?
if(CURSES_FOUND)
    if(CURSES_NCURSES_LIBRARY OR CURSES_HAVE_NCURSES_H OR CURSES_HAVE_NCURSES_NCURSES_H)
        set(HAVE_NCURSES_LIB 1)
    else()
        set(HAVE_CURSES_LIB 1)
    endif()
endif()

configure_file(kernel/acdefs.h.in   ${CMAKE_BINARY_DIR}/include/acdefs.h)

set(XCONQ_VERSION_MAIN "${PROJECT_VERSION}")
configure_file(kernel/version.h.in  ${CMAKE_BINARY_DIR}/include/version.h)

# ---------------------------------------------------------------------------
# Common build settings.
#
# Historical note: the old autoconf build compiled every .c file with the
# C++ compiler (the sources rely on that; kernel headers declare C++
# namespaces). The sources were renamed .cc so the compiler-as-C++ choice
# is now implicit in the filename, rather than forced via a LANGUAGE CXX
# source property.
# ---------------------------------------------------------------------------

# Pinned to 17: the newest standard the codebase gains anything from: the
# earlier gnu++98 pin dates to before the min/max macro fix, which removed
# the only known header clash blocking newer standards. Not pinned newer
# than 17 since the code doesn't use anything from C++20+.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_library(xconq_common INTERFACE)
target_include_directories(xconq_common INTERFACE
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel)
target_compile_definitions(xconq_common INTERFACE HAVE_ACDEFS_H)

# Warnings.  -Wall -Wextra are informational only; a curated subset is
# promoted to hard errors below (real bugs when they fire: bad returns,
# uninitialized reads, format-string mismatches, unintended fallthrough).
# Everything else stays a plain warning for future burn-down passes.
target_compile_options(xconq_common INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=return-type;-Werror=format;-Werror=format-security;-Werror=implicit-fallthrough;-Werror=uninitialized>
    # -Werror=format promotes the whole "format" group in GCC; the curated
    # subset is only format-string/argument correctness and format-security,
    # so keep the buffer-sizing sub-warnings (overflow/truncation) as plain
    # warnings for a later pass.
    $<$<CXX_COMPILER_ID:GNU>:-Wno-error=format-overflow;-Wno-error=format-truncation>
    # -Wmaybe-uninitialized is GCC-only and fires mainly under optimization.
    $<$<CXX_COMPILER_ID:GNU>:-Werror=maybe-uninitialized>)

# Generate a man page by splicing kernel/cmdline.6in into a .6in template
# (replaces the old "sed | move-if-change" rule).
function(xconq_manpage output input)
    file(READ ${input} _page)
    file(READ ${CMAKE_SOURCE_DIR}/kernel/cmdline.6in _cmdline)
    string(REPLACE ".so ../kernel/cmdline.6in\n" "${_cmdline}" _page "${_page}")
    file(WRITE ${output} "${_page}")
endfunction()

# ---------------------------------------------------------------------------
# Subdirectories.
# ---------------------------------------------------------------------------

add_subdirectory(kernel)

if(XCONQ_UI_CURSES AND CURSES_FOUND)
    add_subdirectory(curses)
endif()
if(XCONQ_UI_SDL AND SDL2_FOUND AND X11_FOUND)
    add_subdirectory(sdl)
endif()

add_subdirectory(doc)

enable_testing()
add_subdirectory(test)

# ---------------------------------------------------------------------------
# Game data.
# ---------------------------------------------------------------------------

install(DIRECTORY lib/
        DESTINATION ${XCONQ_DATA_DIR}/lib
        FILES_MATCHING PATTERN "*.g" PATTERN "*.imf" PATTERN "*.dir"
                       PATTERN "news.txt")
install(DIRECTORY images/
        DESTINATION ${XCONQ_DATA_DIR}/images
        FILES_MATCHING PATTERN "*.gif" PATTERN "*.bmp")
