find_package(Threads REQUIRED)

if(BUILD_STATIC)
    set(LIBTYPE STATIC)
    set(CRASY_STATIC 1)
else()
    set(LIBTYPE SHARED)
endif()

if(ENABLE_SSL)
    find_package(OpenSSL REQUIRED)
    set(CRASY_HAS_SSL 1)
endif()

configure_file(config.hpp.in "${OUTPUT_INCLUDEDIR}/crasy/config.hpp")

set(HEADER_DIR "${PROJECT_SOURCE_DIR}/include/crasy")
add_library(crasy ${LIBTYPE}
    config.hpp.in
    "${OUTPUT_INCLUDEDIR}/crasy/config.hpp"

    "${HEADER_DIR}/condition_variable.hpp"
    "${HEADER_DIR}/crasy.hpp"
    "${HEADER_DIR}/detail.hpp"
    "${HEADER_DIR}/endpoint.hpp"
    "${HEADER_DIR}/executor.hpp"
    "${HEADER_DIR}/future.hpp"
    "${HEADER_DIR}/ip_address.hpp"
    "${HEADER_DIR}/lfqueue.hpp"
    "${HEADER_DIR}/lock_guard.hpp"
    "${HEADER_DIR}/mutex.hpp"
    "${HEADER_DIR}/option.hpp"
    "${HEADER_DIR}/shared_mutex.hpp"
    "${HEADER_DIR}/sleep.hpp"
    "${HEADER_DIR}/spawn.hpp"
    "${HEADER_DIR}/spawn_blocking.hpp"
    "${HEADER_DIR}/stream.hpp"
    "${HEADER_DIR}/udp.hpp"
    "${HEADER_DIR}/unique_lock.hpp"
    "${HEADER_DIR}/utils.hpp"

    asio.cpp
    condition_variable.cpp
    executor.cpp
    ip_address.cpp
    mutex.cpp
    shared_mutex.cpp
    udp.cpp
    utils.cpp
)
target_link_libraries(crasy PRIVATE crasy::warnings)
target_link_libraries(crasy PUBLIC asio)
target_link_libraries(crasy PUBLIC ${CMAKE_THREAD_LIBS_INIT})
if(ENABLE_SSL)
    target_link_libraries(crasy PUBLIC OpenSSL::SSL)
endif()
target_include_directories(crasy PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${OUTPUT_INCLUDEDIR}>
    $<INSTALL_INTERFACE:include>
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(crasy PUBLIC -fcoroutines)
endif()
if(NOT BUILD_STATIC)
    target_compile_definitions(crasy PRIVATE -DCRASY_EXPORT)
endif()
add_library(crasy::crasy ALIAS crasy)
