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
include(CheckCXXSymbolExists)
include(CheckCXXSourceCompiles)
include(CheckTypeSize)

# ---------------------------------------------------------------------------
# User-facing options.
# ---------------------------------------------------------------------------

option(XCONQ_UI_CURSES "Build the curses interface (cconq)" ON)
option(XCONQ_UI_SDL    "Build the SDL 1.2 interface (sdlconq)" ON)

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
    find_package(SDL)
    if(NOT SDL_FOUND)
        message(WARNING "SDL 1.2 not found (try sdl12-compat); skipping the SDL interface")
    endif()
    find_package(X11)
endif()

# ---------------------------------------------------------------------------
# Platform introspection feeding the generated acdefs.h.
# ---------------------------------------------------------------------------

check_include_file_cxx(dirent.h HAVE_DIRENT_H)

check_cxx_source_compiles("
#include <sys/time.h>
int main() { struct timeval tv; (void)tv; return 0; }" HAVE_STRUCT_TIMEVAL)
if(NOT HAVE_STRUCT_TIMEVAL)
    set(NEED_STRUCT_TIMEVAL 1)
endif()

check_cxx_source_compiles("
#include <sys/time.h>
int main() { struct timezone tz; (void)tz; return 0; }" HAVE_STRUCT_TIMEZONE)
if(NOT HAVE_STRUCT_TIMEZONE)
    set(NEED_STRUCT_TIMEZONE 1)
endif()

check_cxx_symbol_exists(gettimeofday "sys/time.h" HAVE_GETTIMEOFDAY)
if(NOT HAVE_GETTIMEOFDAY)
    set(NEED_GETTIMEOFDAY 1)
endif()

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
if(XCONQ_UI_SDL AND SDL_FOUND AND X11_FOUND)
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
