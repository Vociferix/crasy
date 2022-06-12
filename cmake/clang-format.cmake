if(ENABLE_CLANG_FORMAT)
    if(NOT CLANG_FORMAT)
        find_program(_CLANG_FORMAT 
            NAMES clang-format clang-format-12
        )
        if(_CLANG_FORMAT)
            override_config_option(CLANG_FORMAT "${_CLANG_FORMAT}")
        else()
            message(STATUS "clang-format not found - formatting disabled")
        endif()
        mark_as_advanced(_CLANG_FORMAT)
    endif()

    if(CLANG_FORMAT)
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/format")
        macro(clang_format)
            set(_FILES)
            foreach(_ENTRY ${ARGN})
                if(NOT IS_ABSOLUTE "${_ENTRY}")
                    set(_ENTRY "${CMAKE_CURRENT_SOURCE_DIR}/${_ENTRY}")
                endif()
                if(IS_DIRECTORY "${_ENTRY}")
                    file(GLOB_RECURSE _DIR_FILES
                        "${_ENTRY}/*.c"
                        "${_ENTRY}/*.cpp"
                        "${_ENTRY}/*.cxx"
                        "${_ENTRY}/*.cc"
                        "${_ENTRY}/*.c++"
                        "${_ENTRY}/*.C"
                        "${_ENTRY}/*.h"
                        "${_ENTRY}/*.hpp"
                        "${_ENTRY}/*.hxx"
                        "${_ENTRY}/*.hh"
                        "${_ENTRY}/*.h++"
                        "${_ENTRY}/*.H"
                        "${_ENTRY}/*.inl"
                        "${_ENTRY}/*.c.in"
                        "${_ENTRY}/*.cpp.in"
                        "${_ENTRY}/*.cxx.in"
                        "${_ENTRY}/*.cc.in"
                        "${_ENTRY}/*.c++.in"
                        "${_ENTRY}/*.C.in"
                        "${_ENTRY}/*.h.in"
                        "${_ENTRY}/*.hpp.in"
                        "${_ENTRY}/*.hxx.in"
                        "${_ENTRY}/*.hh.in"
                        "${_ENTRY}/*.h++.in"
                        "${_ENTRY}/*.H.in"
                        "${_ENTRY}/*.inl.in"
                    )
                    list(APPEND _FILES ${_DIR_FILES})
                else()
                    list(APPEND _FILES "${_ENTRY}")
                endif()
            endforeach()
            set(_TAG_FILES)
            foreach(_FILE ${_FILES})
                file(RELATIVE_PATH _REL_FILE "${PROJECT_SOURCE_DIR}" "${_FILE}")
                set(_TAG_FILE "${PROJECT_BINARY_DIR}/format/${_REL_FILE}")
                list(APPEND _TAG_FILES "${_TAG_FILE}")
                get_filename_component(_TAG_FILE_DIR "${_TAG_FILE}" DIRECTORY)
                file(MAKE_DIRECTORY "${_TAG_FILE_DIR}")
                add_custom_command(OUTPUT "${_TAG_FILE}"
                    COMMAND ${CLANG_FORMAT} -style=file -i "${_FILE}"
                    COMMAND ${CMAKE_COMMAND} -E touch "${_TAG_FILE}"
                    DEPENDS "${_FILE}"
                    COMMENT "clang-format ${_REL_FILE}"
                )
            endforeach()
            add_custom_target(format DEPENDS ${_TAG_FILES})
        endmacro()
    else()
        macro(clang_format)
        endmacro()
    endif()
else()
    macro(clang_format)
    endmacro()
endif()
