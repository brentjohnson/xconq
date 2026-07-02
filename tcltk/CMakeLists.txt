# CMake build for the Tcl/Tk interface to Xconq.
#
# This is the primary interface, so the executable is installed as "xconq"
# (matching the old build's default-UI behavior).

set(TKCONQ_SOURCES
    tkunix.c
    tkmain.c
    tkinit.c
    tkcmd.c
    tkmap.c
    # image-family support (was libtkimf.a)
    tkisamp.c
    tkimf.c)

set(IMFAPP_SOURCES
    iappunix.c
    tkiapp.c
    tkisamp.c
    tkimf.c
    ${CMAKE_SOURCE_DIR}/x11/xcutil.c)

xconq_cxx_sources(${TKCONQ_SOURCES} ${IMFAPP_SOURCES})

set(TCLTK_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${TCL_INCLUDE_PATH}
    ${TK_INCLUDE_PATH}
    ${X11_INCLUDE_DIR})
set(TCLTK_X11_LIBS X11::Xext X11::Xmu X11::X11)

add_executable(xconq ${TKCONQ_SOURCES})
target_include_directories(xconq PRIVATE ${TCLTK_INCLUDES})
target_link_libraries(xconq PRIVATE
    conq ${TK_LIBRARY} ${TCL_LIBRARY} ${TCLTK_X11_LIBS} m)

# Standalone image-family editor.
add_executable(imfapp ${IMFAPP_SOURCES})
target_include_directories(imfapp PRIVATE ${TCLTK_INCLUDES})
target_link_libraries(imfapp PRIVATE
    conqlow ${TK_LIBRARY} ${TCL_LIBRARY} ${TCLTK_X11_LIBS} m)

xconq_manpage(${CMAKE_CURRENT_BINARY_DIR}/xconq.6 ${CMAKE_SOURCE_DIR}/x11/xconq.6in)

install(TARGETS xconq imfapp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xconq.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)

# Tcl scripts and the bundled BWidget toolkit, loaded at run time.
install(FILES tkconq.tcl imfapp.tcl DESTINATION ${XCONQ_DATA_DIR}/tcltk)
install(DIRECTORY BWidget DESTINATION ${XCONQ_DATA_DIR}/tcltk)
