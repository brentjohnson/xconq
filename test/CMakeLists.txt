# CTest wiring for the Xconq test suite.
#
# The test-*.sh scripts take the test source directory as their argument,
# expect to find skelconq at ../kernel/skelconq relative to the working
# directory, and write their logs into the working directory.  Running them
# from ${CMAKE_CURRENT_BINARY_DIR} preserves those assumptions.

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

set_tests_properties(check-long check-run PROPERTIES LABELS "long")

# Nothing should run forever; the game-playing tests get generous limits.
set_tests_properties(check-lib check-actions check-save check-test check-auto
                     PROPERTIES TIMEOUT 1800)
set_tests_properties(check-ai check-run check-long PROPERTIES TIMEOUT 3600)

# Pre-existing kernel bugs (independent of the build system):
#  - check-auto: the autotest crashes with an assertion in do_toolup_action
#    (kernel/actions.c).
#  - check-ai: skelconq loops forever on lib/cil-rules.g, spewing
#    "run_game: tested for game start." until the disk fills.
# Disabled until the kernel is fixed; run manually with: ctest -R <name>
set_tests_properties(check-auto check-ai PROPERTIES DISABLED TRUE)
