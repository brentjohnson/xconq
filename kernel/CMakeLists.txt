# CMake build for the interface-independent Xconq kernel.
#
# Xconq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# The high-level kernel library, linked with interface code.
set(CONQ_SOURCES
    init.cc
    cmdline.cc
    aiutil.cc
    ai.cc
    aiscore.cc
    aiunit.cc
    aiunit2.cc
    aitact.cc
    aioprt.cc
    mplayer.cc
    actions.cc
    move.cc
    combat.cc
    run.cc
    run2.cc
    economy.cc
    supply.cc
    score.cc
    world.cc
    history.cc
    module.cc
    write.cc
    read.cc
    mkterr.cc
    mkunits.cc
    mkrivers.cc
    mkroads.cc
    mknames.cc
    side.cc
    unit.cc
    plan.cc
    task.cc
    nlang.cc
    ui.cc
    cmd.cc
    ps.cc
    tp.cc
    help.cc
    copying.cc
    generic.cc
    types.cc
    tables.cc)

# The low-level kernel library, linked with both complete games and
# GDL-using tools.
set(CONQLOW_SOURCES
    imf.cc
    gif.cc
    lisp.cc
    util.cc)
if(WIN32)
    list(APPEND CONQLOW_SOURCES win32.c socket.cc)
else()
    list(APPEND CONQLOW_SOURCES unix.cc socket.cc)
endif()

add_library(conqlow STATIC ${CONQLOW_SOURCES})
target_link_libraries(conqlow PUBLIC xconq_common)
# Compiled-in default search paths.
target_compile_definitions(conqlow PRIVATE
    XCONQDATA="${XCONQ_DATA_DIR}"
    XCONQSCORES="${XCONQ_SCORES_DIR}")

add_library(conq STATIC ${CONQ_SOURCES})
target_link_libraries(conq PUBLIC conqlow)

# Skeleton (headless) interface; drives the test suite.
add_executable(skelconq skelconq.cc autotest.cc)
target_link_libraries(skelconq PRIVATE conq m)

# IMF <-> IMF converter tool (build on demand).
add_executable(imf2imf EXCLUDE_FROM_ALL imf2imf.cc)
target_link_libraries(imf2imf PRIVATE conqlow)

# Documentation writer (build on demand).
add_executable(xcscribe EXCLUDE_FROM_ALL xcscribe.cc)
target_link_libraries(xcscribe PRIVATE conq m)
