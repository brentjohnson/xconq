# CMake build for the legacy Xt/Xaw interface to Xconq (xtconq) and the
# X11 image-conversion tools.

# App-defaults resources compiled into the binary (was the Ad2c rule).
execute_process(
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/Ad2c/ad2c.script
            ${CMAKE_CURRENT_SOURCE_DIR}/Xtconq.ad
    OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/xtconqad.h
    RESULT_VARIABLE _ad2c_result)
if(_ad2c_result)
    message(FATAL_ERROR "ad2c failed to generate xtconqad.h")
endif()

set(XTCONQ_SOURCES
    xtconq.c
    xtmain.c
    xinit.c
    xmap.c
    xdraw.c
    xcloseup.c
    xcmd.c
    xprint.c
    xdesign.c
    xhelp.c
    ximf.c)

set(X11_TOOL_COMMON ximf.c xcutil.c)

xconq_cxx_sources(${XTCONQ_SOURCES} ${X11_TOOL_COMMON}
    x2imf.c imf2x.c xshowimf.c)

set(XAW_LIBS X11::Xaw X11::Xt X11::Xmu X11::Xext X11::X11 m)

add_executable(xtconq ${XTCONQ_SOURCES})
target_include_directories(xtconq PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR} ${X11_INCLUDE_DIR})
target_link_libraries(xtconq PRIVATE conq ${XAW_LIBS})

# X bitmap/pixmap <-> IMF converters.
foreach(tool x2imf imf2x)
    add_executable(${tool} ${tool}.c ${X11_TOOL_COMMON})
    target_include_directories(${tool} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${X11_INCLUDE_DIR})
    target_link_libraries(${tool} PRIVATE conqlow ${XAW_LIBS})
endforeach()

# IMF viewer, with the bundled SelFile file-selection widget.
# SelFile is K&R-era C; pin it to C90 so modern compilers accept it.
add_library(Xgnu STATIC
    SelFile/SelFile.c SelFile/Path.c SelFile/Dir.c SelFile/Draw.c)
set_target_properties(Xgnu PROPERTIES C_STANDARD 90 C_EXTENSIONS ON)
target_compile_definitions(Xgnu PRIVATE SELFILE HOME_ON_DEMAND HAVE_ACDEFS_H)
target_include_directories(Xgnu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/SelFile ${CMAKE_BINARY_DIR}/include
    ${X11_INCLUDE_DIR})

add_executable(xshowimf xshowimf.c ${X11_TOOL_COMMON})
target_compile_definitions(xshowimf PRIVATE HAVE_SELFILE)
target_include_directories(xshowimf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${X11_INCLUDE_DIR})
target_link_libraries(xshowimf PRIVATE conqlow Xgnu ${XAW_LIBS})

xconq_manpage(${CMAKE_CURRENT_BINARY_DIR}/xtconq.6 ${CMAKE_CURRENT_SOURCE_DIR}/xtconq.6in)

install(TARGETS xtconq x2imf imf2x xshowimf RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xtconq.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)

# X resource files (monochrome and color variants).
install(FILES Xtconq.ad RENAME Xtconq
        DESTINATION ${CMAKE_INSTALL_DATADIR}/X11/app-defaults)
install(FILES Xtconq-co.ad RENAME Xtconq-color
        DESTINATION ${CMAKE_INSTALL_DATADIR}/X11/app-defaults)
