# CTest wiring for the Xconq test suite.
#
# The test-*.sh scripts take the test source directory as their argument,
# expect to find skelconq at ../kernel/skelconq relative to the working
# directory (or at $SKELCONQ, if set), and write their logs into the
# working directory. The aggregate/manual scripts run from
# ${CMAKE_CURRENT_BINARY_DIR}, which preserves the relative-path
# assumption; the per-module tests below instead get their own scratch
# directory and an absolute $SKELCONQ, since they run concurrently under
# `ctest -j` and cannot share a working directory.
#
# The scripts exit nonzero when a game crashes, or when a playable game
# (one listed in lib/game.dir) produces Error:/Warning: output, fails to
# save, or fails the save/restore comparison; see test/common.sh for the
# full policy. They keep saves and scores out of ~/.xconq by pointing
# XCONQHOME into their working directory.

set(XCONQ_TEST_SCRIPTS
    ai:test-ai.sh
    run:test-run.sh
    long:test-long.sh)

foreach(entry IN LISTS XCONQ_TEST_SCRIPTS)
    string(REPLACE ":" ";" entry ${entry})
    list(GET entry 0 name)
    list(GET entry 1 script)
    add_test(NAME check-${name}
             COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/${script} ${CMAKE_CURRENT_SOURCE_DIR}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

add_test(NAME check-auto
         COMMAND $<TARGET_FILE:skelconq> --auto
                 -f ${CMAKE_CURRENT_SOURCE_DIR}/auto.g
                 -L ${CMAKE_SOURCE_DIR}/lib
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# check-lib, check-actions, check-save, and check-test run one CTest per
# game module instead of one aggregate sweep, so ctest -j can run the
# modules in parallel and a failure names the exact .g file that broke.
# Each module gets its own scratch working directory, so the XCONQHOME,
# log, and save files the scripts write via relative/cwd-based paths
# can't collide between tests running at the same time; skelconq's
# absolute path is passed through the environment since these tests
# don't run from ${CMAKE_CURRENT_BINARY_DIR}.

file(GLOB XCONQ_LIB_MODULES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/lib/*.g)
file(GLOB XCONQ_TEST_MODULES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.g)

# All test timeouts are scaled by XCONQ_TEST_TIMEOUT_SCALE (default 1) so the
# slower sanitized CI leg can widen them without touching normal builds. The
# same factor is handed to the scripts as XCONQ_TIMEOUT_SCALE so test/common.sh
# scales its own per-game `timeout` bound to match.
math(EXPR XCONQ_MODULE_TIMEOUT "600 * ${XCONQ_TEST_TIMEOUT_SCALE}")
math(EXPR XCONQ_AUTO_TIMEOUT "1800 * ${XCONQ_TEST_TIMEOUT_SCALE}")
math(EXPR XCONQ_LONG_TIMEOUT "14400 * ${XCONQ_TEST_TIMEOUT_SCALE}")

function(xconq_add_module_tests prefix label script)
    foreach(module ${ARGN})
        get_filename_component(modname ${module} NAME_WE)
        set(scratch ${CMAKE_CURRENT_BINARY_DIR}/scratch/${prefix}-${modname})
        file(MAKE_DIRECTORY ${scratch})
        add_test(NAME ${prefix}-${modname}
                 COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/${script}
                         ${CMAKE_CURRENT_SOURCE_DIR} ${module}
                 WORKING_DIRECTORY ${scratch})
        set_tests_properties(${prefix}-${modname} PROPERTIES
                              LABELS ${label}
                              TIMEOUT ${XCONQ_MODULE_TIMEOUT}
                              ENVIRONMENT "SKELCONQ=$<TARGET_FILE:skelconq>;XCONQ_TIMEOUT_SCALE=${XCONQ_TEST_TIMEOUT_SCALE}")
    endforeach()
endfunction()

xconq_add_module_tests(check-lib lib test-lib.sh ${XCONQ_LIB_MODULES})
xconq_add_module_tests(check-actions actions test-acts.sh ${XCONQ_LIB_MODULES})
xconq_add_module_tests(check-save save test-save.sh ${XCONQ_LIB_MODULES})
xconq_add_module_tests(check-test test test-test.sh ${XCONQ_TEST_MODULES})

# The AI self-play sweep legitimately needs hours for the biggest games,
# so it lives in the "long" lane with the full-game runs; the quick lane
# is check-lib/actions/save/test (one CTest per module) plus check-auto.
set_tests_properties(check-ai check-long check-run PROPERTIES LABELS "long")

# Nothing should run forever; the game-playing tests get generous limits.
# The scripts also bound each individual game run (10 minutes, 100 MB of
# output) so one hung game cannot eat a whole test's budget or the disk.
# The per-module lib/actions/save/test tests get TIMEOUT 600 above,
# matching that same per-run bound; check-auto is a single fixed run.
set_tests_properties(check-auto PROPERTIES TIMEOUT ${XCONQ_AUTO_TIMEOUT})
set_tests_properties(check-ai check-run check-long PROPERTIES TIMEOUT ${XCONQ_LONG_TIMEOUT})
