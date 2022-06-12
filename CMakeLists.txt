cmake_minimum_required(VERSION 3.12)
project(crasy LANGUAGES CXX)

include(cmake/settings.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third-party)
add_subdirectory(src)
add_subdirectory(test)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

clang_format(include src)
include(cmake/doxygen.cmake)
