config_option(DEVEL BOOL "Enable crasy developer options by default" OFF)
config_option(WERROR BOOL "Treat all crasy compiler warnings as errors" ${DEVEL})
config_option(ENABLE_CLANG_FORMAT BOOL "Enable crasy code formatting with clang-format" ${DEVEL})
config_option(BUILD_STATIC BOOL "Build crasy as a static library" OFF)
config_option(ENABLE_SSL BOOL "Enable SSL/TLS support" ON)
config_option(BUILD_EXAMPLES BOOL "Build crasy examples" ${DEVEL})

config_option(OUTPUT_DIR STRING "Output directory for crasy compile binaries and generated files" "${PROJECT_BINARY_DIR}/output")

if(ENABLE_CLANG_FORMAT)
    config_option(CLANG_FORMAT STRING "Path to clang-format executable (automatically detected if not set)")
endif()
