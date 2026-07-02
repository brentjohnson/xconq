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

option(XCONQ_UI_TCLTK  "Build the Tcl/Tk interface (installed as 'xconq')" ON)
option(XCONQ_UI_CURSES "Build the curses interface (cconq)" ON)
option(XCONQ_UI_SDL    "Build the SDL 1.2 interface (sdlconq)" ON)
option(XCONQ_UI_X11    "Build the legacy Xt/Xaw interface (xtconq) and X11 image tools" ON)

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

if(XCONQ_UI_TCLTK)
    find_package(TCL)
    if(NOT TCLTK_FOUND)
        message(WARNING "Tcl/Tk not found; skipping the Tcl/Tk interface")
    elseif(TCL_INCLUDE_PATH)
        # The UI code uses the Tcl/Tk 8.x C API (Tk_Offset, string-based
        # Tk_ConfigureWidget, ...), which Tcl/Tk 9 removed.
        file(STRINGS ${TCL_INCLUDE_PATH}/tcl.h _tcl_major
             REGEX "define[ \t]+TCL_MAJOR_VERSION")
        if(_tcl_major MATCHES "TCL_MAJOR_VERSION[ \t]+([0-9]+)"
           AND CMAKE_MATCH_1 GREATER 8)
            message(WARNING "Tcl/Tk ${CMAKE_MATCH_1}.x found, but the Tcl/Tk "
                    "interface requires 8.x; skipping it")
            set(TCLTK_FOUND FALSE)
        endif()
    endif()
endif()

if(XCONQ_UI_SDL)
    find_package(SDL)
    if(NOT SDL_FOUND)
        message(WARNING "SDL 1.2 not found (try sdl12-compat); skipping the SDL interface")
    endif()
endif()

if(XCONQ_UI_TCLTK OR XCONQ_UI_SDL OR XCONQ_UI_X11)
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
# C++ compiler.  The sources rely on that (kernel headers declare C++
# namespaces), so C sources in xconq targets are marked LANGUAGE CXX, and
# -fpermissive papers over pre-standard idioms.
# ---------------------------------------------------------------------------

# The sources predate C++11; newer standards make libstdc++ headers clash
# with the kernel's min/max macros (via <math.h> in C++17+).
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS ON)

add_library(xconq_common INTERFACE)
target_include_directories(xconq_common INTERFACE
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel)
target_compile_definitions(xconq_common INTERFACE HAVE_ACDEFS_H)
target_compile_options(xconq_common INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fpermissive>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wno-write-strings>)

# Mark a target's C sources for compilation as C++ (see note above).
function(xconq_cxx_sources)
    set_source_files_properties(${ARGN} PROPERTIES LANGUAGE CXX)
endfunction()

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
if(XCONQ_UI_TCLTK AND TCLTK_FOUND AND X11_FOUND)
    add_subdirectory(tcltk)
endif()
if(XCONQ_UI_SDL AND SDL_FOUND AND X11_FOUND)
    add_subdirectory(sdl)
endif()
if(XCONQ_UI_X11 AND X11_FOUND AND X11_Xaw_FOUND)
    add_subdirectory(x11)
elseif(XCONQ_UI_X11)
    message(WARNING "X11/Xaw not found; skipping the xtconq interface")
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

if(UNIX AND NOT APPLE)
    install(FILES xconq.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES xconq.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
endif()
