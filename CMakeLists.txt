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

# ---------------------------------------------------------------------------
# User-facing options.
# ---------------------------------------------------------------------------

option(XCONQ_UI_SDL    "Build the SDL 3 interface (sdlconq)" ON)

set(XCONQ_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/xconq" CACHE PATH
    "Installed location of the games library, images, and UI scripts")
set(XCONQ_SCORES_DIR "" CACHE PATH
    "Directory used for score files at run time, for shared installs. \
Empty (the default) means per-user scores under the XDG data dir instead")

# Comma-separated -fsanitize list (e.g. "address,undefined"); empty disables.
# Wired into every kernel/UI target via xconq_common below. Intended for a
# dedicated CI leg on RelWithDebInfo; see .github/workflows/c-cpp.yml.
set(XCONQ_SANITIZE "" CACHE STRING
    "Comma-separated -fsanitize list (e.g. address,undefined); empty disables")

# Multiplies the per-test CTest TIMEOUT and the per-game bound in
# test/common.sh. Sanitized runs are 2-5x slower, so the sanitizer CI leg
# configures with e.g. -DXCONQ_TEST_TIMEOUT_SCALE=3; normal builds leave it 1.
set(XCONQ_TEST_TIMEOUT_SCALE 1 CACHE STRING
    "Integer multiplier for test timeouts (raise for slow sanitized runs)")

# Opt-in libFuzzer harness for the GDL reader (Clang only). When ON, builds a
# dedicated fuzz_gdl target under test/fuzz/ with -fsanitize=fuzzer,address,
# undefined on the reader objects it links. Kept out of default builds; see
# test/fuzz/README.md. A short CI smoke-fuzz leg turns this on.
option(XCONQ_FUZZ "Build the libFuzzer GDL-reader harness (Clang only)" OFF)

# ---------------------------------------------------------------------------
# Dependency discovery.  Each UI is skipped (with a warning) when its
# dependencies are missing, so a bare kernel build always works.
# ---------------------------------------------------------------------------

if(XCONQ_UI_SDL)
    find_package(SDL3)
    if(NOT SDL3_FOUND)
        message(WARNING "SDL3 not found (try libsdl3-dev); skipping the SDL interface")
    endif()
    find_package(X11)
endif()

# ---------------------------------------------------------------------------
# Platform introspection feeding the generated acdefs.h.
# ---------------------------------------------------------------------------

check_include_file_cxx(dirent.h HAVE_DIRENT_H)

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

# Sanitizers.  Applied to compile *and* link of everything that links
# xconq_common (kernel, tools, both UIs).  -fno-omit-frame-pointer keeps
# ASan/UBSan backtraces readable; -g gives file:line.  RelWithDebInfo is the
# intended build type (optimized enough to be tolerable, still debuggable).
if(XCONQ_SANITIZE)
    target_compile_options(xconq_common INTERFACE
        -fsanitize=${XCONQ_SANITIZE} -fno-omit-frame-pointer -g)
    target_link_options(xconq_common INTERFACE
        -fsanitize=${XCONQ_SANITIZE})
endif()

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

if(XCONQ_UI_SDL AND SDL3_FOUND AND X11_FOUND)
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
