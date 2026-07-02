# CMake build for the interface-independent Xconq kernel.
#
# Xconq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# The high-level kernel library, linked with interface code.
set(CONQ_SOURCES
    init.c
    cmdline.c
    aiutil.c
    ai.c
    aiscore.cc
    aiunit.cc
    aiunit2.cc
    aitact.cc
    aioprt.cc
    mplayer.c
    actions.c
    move.c
    combat.c
    run.c
    run2.c
    economy.c
    supply.c
    score.c
    world.c
    history.c
    module.c
    write.c
    read.c
    mkterr.c
    mkunits.c
    mkrivers.c
    mkroads.c
    mknames.c
    side.c
    unit.c
    plan.c
    task.c
    nlang.c
    ui.c
    cmd.c
    ps.c
    tp.c
    obstack.c
    help.c
    copying.c
    generic.c
    types.c
    tables.c)

# The low-level kernel library, linked with both complete games and
# GDL-using tools.
set(CONQLOW_SOURCES
    imf.c
    gif.c
    lisp.c
    util.c
    snprintf.c
    timestuff.c)
if(WIN32)
    list(APPEND CONQLOW_SOURCES win32.c socket.c)
else()
    list(APPEND CONQLOW_SOURCES unix.c socket.c)
endif()

xconq_cxx_sources(${CONQ_SOURCES} ${CONQLOW_SOURCES}
    skelconq.c autotest.c imf2imf.c xcscribe.c)

add_library(conqlow STATIC ${CONQLOW_SOURCES})
target_link_libraries(conqlow PUBLIC xconq_common)
# Compiled-in default search paths.
target_compile_definitions(conqlow PRIVATE
    XCONQDATA="${XCONQ_DATA_DIR}"
    XCONQSCORES="${XCONQ_SCORES_DIR}")

add_library(conq STATIC ${CONQ_SOURCES})
target_link_libraries(conq PUBLIC conqlow)

# Skeleton (headless) interface; drives the test suite.
add_executable(skelconq skelconq.c autotest.c)
target_link_libraries(skelconq PRIVATE conq m)

# IMF <-> IMF converter tool (build on demand).
add_executable(imf2imf EXCLUDE_FROM_ALL imf2imf.c)
target_link_libraries(imf2imf PRIVATE conqlow)

# Documentation writer (build on demand).
add_executable(xcscribe EXCLUDE_FROM_ALL xcscribe.c)
target_link_libraries(xcscribe PRIVATE conq m)
