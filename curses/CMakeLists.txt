# CMake build for the curses interface to Xconq (cconq).

set(CCONQ_SOURCES cconq.cc cdraw.cc ccmd.cc)

add_executable(cconq ${CCONQ_SOURCES})
target_include_directories(cconq PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(cconq PRIVATE conq ${CURSES_LIBRARIES} m)

xconq_manpage(${CMAKE_CURRENT_BINARY_DIR}/cconq.6 ${CMAKE_CURRENT_SOURCE_DIR}/cconq.6in)

install(TARGETS cconq RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cconq.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
