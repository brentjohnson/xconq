# libFuzzer harness for the GDL reader.  Opt-in (-DXCONQ_FUZZ=ON), Clang only,
# kept out of default builds.  See test/fuzz/README.md.
#
# Xconq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

if(NOT XCONQ_FUZZ)
    return()
endif()

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
        "XCONQ_FUZZ needs Clang for libFuzzer "
        "(-fsanitize=fuzzer); configure with CXX=clang++.")
endif()

# fuzzer = coverage instrumentation + libFuzzer main (linked into the exe);
# address,undefined catch the memory-safety / UB bugs we are hunting.  Applied
# to compile *and* link of every object in the fuzz target so instrumentation
# is uniform.
set(XCONQ_FUZZ_FLAGS -fsanitize=fuzzer,address,undefined
                     -fno-omit-frame-pointer -g)

# The low-level reader objects (kernel/conqlow set: lisp/util + platform glue),
# built here with fuzz instrumentation rather than reusing the plain `conqlow`
# library.  Only the objects the harness actually references get pulled into
# the executable from this static lib (imf/gif/socket stay out), exactly as
# imf2imf links the same low-level layer standalone.  Keep in sync with
# CONQLOW_SOURCES in kernel/CMakeLists.txt.
set(_reader_srcs
    ${CMAKE_SOURCE_DIR}/kernel/lisp.cc
    ${CMAKE_SOURCE_DIR}/kernel/util.cc
    ${CMAKE_SOURCE_DIR}/kernel/unix.cc
    ${CMAKE_SOURCE_DIR}/kernel/socket.cc
    ${CMAKE_SOURCE_DIR}/kernel/imf.cc
    ${CMAKE_SOURCE_DIR}/kernel/gif.cc)

add_library(conqfuzz_reader STATIC ${_reader_srcs})
target_link_libraries(conqfuzz_reader PUBLIC xconq_common)
target_compile_definitions(conqfuzz_reader PRIVATE
    XCONQDATA="${XCONQ_DATA_DIR}"
    XCONQSCORES="${XCONQ_SCORES_DIR}")
target_compile_options(conqfuzz_reader PRIVATE ${XCONQ_FUZZ_FLAGS})

add_executable(fuzz_gdl fuzz_gdl.cc fuzz_stubs.cc)
target_link_libraries(fuzz_gdl PRIVATE conqfuzz_reader xconq_common)
target_compile_options(fuzz_gdl PRIVATE ${XCONQ_FUZZ_FLAGS})
target_link_options(fuzz_gdl PRIVATE ${XCONQ_FUZZ_FLAGS})

# CTest smoke test: replay the whole checked-in corpus under the sanitizers
# (no mutation, -runs=0 just executes each seed and exits nonzero on any
# crash).  This is the regression guard for every reader bug the fuzzer found.
# The corpus dir is passed as a directory argument; libFuzzer runs each file.
add_test(NAME fuzz-gdl-corpus
    COMMAND fuzz_gdl -runs=0 ${CMAKE_CURRENT_SOURCE_DIR}/corpus)
set_tests_properties(fuzz-gdl-corpus PROPERTIES
    LABELS "fuzz"
    TIMEOUT 120
    # detect_leaks=0: the reader interns/allocates and frees nothing by design
    # (same rationale as the sanitizers CI leg), so LSan would flag the corpus.
    # halt_on_error makes any UB finding fail the test.
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0:abort_on_error=1;UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1")
