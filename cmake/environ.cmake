if("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    set(TOP_LEVEL_PROJECT TRUE)
else()
    set(TOP_LEVEL_PROJECT FALSE)
endif()
