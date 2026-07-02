# Texinfo documentation.  Only wired up when makeinfo is available;
# build with "cmake --build . --target info".

find_program(MAKEINFO_EXECUTABLE makeinfo)

if(MAKEINFO_EXECUTABLE)
    set(_infos)
    foreach(manual xconq xcdesign hacking)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${manual}.info
            COMMAND ${MAKEINFO_EXECUTABLE} -I ${CMAKE_CURRENT_SOURCE_DIR}
                    -o ${CMAKE_CURRENT_BINARY_DIR}/${manual}.info
                    ${CMAKE_CURRENT_SOURCE_DIR}/${manual}.texi
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${manual}.texi
            COMMENT "Generating ${manual}.info")
        list(APPEND _infos ${CMAKE_CURRENT_BINARY_DIR}/${manual}.info)
    endforeach()
    add_custom_target(info DEPENDS ${_infos})
else()
    message(STATUS "makeinfo not found; 'info' documentation target disabled")
endif()
