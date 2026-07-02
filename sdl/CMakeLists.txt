# CMake build for the (experimental) SDL 1.2 interface to Xconq (sdlconq).
# On modern systems the SDL 1.2 API is provided by sdl12-compat.

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
    ${CMAKE_SOURCE_DIR}
    ${SDL_INCLUDE_DIR})
target_link_libraries(sdlconq PRIVATE
    conq ${SDL_LIBRARY} X11::Xext X11::Xmu X11::X11 m)

install(TARGETS sdlconq RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
