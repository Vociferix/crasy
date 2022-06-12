if(TOP_LEVEL_PROJECT)
    set(EXAMPLES_TGT "examples")
else()
    set(EXAMPLES_TGT "crasy_examples")
endif()
add_custom_target("${EXAMPLES_TGT}")

macro(add_example CPP_FILE)
    get_filename_component(_NAME "${CPP_FILE}" NAME_WE)
    if(TOP_LEVEL_PROJECT)
        set(_TGT "${_NAME}_example")
    else()
        set(_TGT "crasy_${_NAME}_example")
    endif()
    add_executable("${_TGT}" "${CPP_FILE}")
    target_link_libraries("${_TGT}" PRIVATE
        crasy::crasy
        crasy::warnings
    )
    add_dependencies("${EXAMPLES_TGT}" "${_TGT}")
endmacro()

add_example(udp_echo_server.cpp)
