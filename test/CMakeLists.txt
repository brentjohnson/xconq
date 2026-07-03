# CTest wiring for the Xconq test suite.
#
# The test-*.sh scripts take the test source directory as their argument,
# expect to find skelconq at ../kernel/skelconq relative to the working
# directory, and write their logs into the working directory.  Running them
# from ${CMAKE_CURRENT_BINARY_DIR} preserves those assumptions.
#
# The scripts exit nonzero when a game crashes, or when a playable game
# (one listed in lib/game.dir) produces Error:/Warning: output, fails to
# save, or fails the save/restore comparison; see test/common.sh for the
# full policy.  They keep saves and scores out of ~/.xconq by pointing
# XCONQHOME into the build directory.

set(XCONQ_TEST_SCRIPTS
    lib:test-lib.sh
    actions:test-acts.sh
    save:test-save.sh
    test:test-test.sh
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

# The AI self-play sweep legitimately needs hours for the biggest games,
# so it lives in the "long" lane with the full-game runs; the quick lane
# is check-lib/actions/save/test/auto.
set_tests_properties(check-ai check-long check-run PROPERTIES LABELS "long")

# Nothing should run forever; the game-playing tests get generous limits.
# The scripts also bound each individual game run (10 minutes, 100 MB of
# output) so one hung game cannot eat a whole test's budget or the disk.
set_tests_properties(check-lib check-actions check-save check-test check-auto
                     PROPERTIES TIMEOUT 1800)
set_tests_properties(check-ai check-run check-long PROPERTIES TIMEOUT 14400)
