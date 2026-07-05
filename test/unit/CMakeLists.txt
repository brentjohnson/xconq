# True unit tests for pure-kernel logic (the GDL reader/writer, the .def
# type/table machinery, and small utility helpers), as opposed to the rest
# of test/ which drives whole games through skelconq. See unittests.cc's
# header comment for why this links kernel/skelconq_stubs.cc directly
# rather than the skelconq executable (which also has an interactive
# command loop this doesn't need).

add_executable(unittests unittests.cc ${CMAKE_SOURCE_DIR}/kernel/skelconq_stubs.cc)
target_link_libraries(unittests PRIVATE conq m)

add_test(NAME unittests COMMAND unittests)
set_tests_properties(unittests PROPERTIES LABELS "unit" TIMEOUT 60)
