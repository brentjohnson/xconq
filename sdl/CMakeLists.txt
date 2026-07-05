# CMake build for the SDL 2 interface to Xconq (sdlconq).

set(SDLCONQ_SOURCES
    sdlunix.cc
    sdlmain.cc
    sdlinit.cc
    sdlcmd.cc
    sdlscreen.cc
    sdlmap.cc
    sdlimf.cc
    sdluact.cc
    sdlcltbl.cc)

add_executable(sdlconq ${SDLCONQ_SOURCES})
target_include_directories(sdlconq PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR})
target_link_libraries(sdlconq PRIVATE
    conq SDL2::SDL2 X11::Xext X11::Xmu X11::X11 m)

install(TARGETS sdlconq RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
