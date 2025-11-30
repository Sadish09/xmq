cmake_minimum_required(VERSION 3.16)
project(XMQ VERSION 0.1 LANGUAGES C CXX)


# Top-level build options (user-configurable)

option(ENABLE_LIBOQS "Enable liboqs (post-quantum primitives)" ON)
option(LIBOQS_USE_SUBMODULE "Use liboqs from local path (OQS_DIR) instead of FetchContent" OFF)
option(USE_OQS_OPENSSL "Link against OQS-OpenSSL (hybrid TLS) - requires OQS_OPENSSL_DIR" OFF)
option(ENABLE_APP_PQC_PAYLOAD "Enable application-layer PQC payload wrapping" ON)
option(ENABLE_TESTS "Build unit and integration tests" ON)

set(LIBOQS_GIT_TAG "v0.9.1" CACHE STRING "liboqs git tag/commit to fetch via FetchContent")
set(OQS_DIR "" CACHE PATH "If set and LIBOQS_USE_SUBMODULE=ON, use this path for liboqs source")
set(OQS_OPENSSL_DIR "" CACHE PATH "Path to OQS-OpenSSL build or install (if USE_OQS_OPENSSL=ON)")


# Include helper toolchain & cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(cmake/BaseToolchain.cmake)       # defines common flags & helper functions
include(cmake/FindOQS.cmake)            # attempt to locate liboqs in system

# project-wide settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Detect architecture for compile-time defs
message(STATUS "System processor: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i386|i686")
    set(ARCH_X86 TRUE)
    add_definitions(-DARCH_X86)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|armv7l|armv7")
    set(ARCH_ARM TRUE)
    add_definitions(-DARCH_ARM)
endif()


# Fetch liboqs along with git submodule support
if(ENABLE_LIBOQS)
    if(LIBOQS_USE_SUBMODULE)
        if(NOT OQS_DIR)
            message(FATAL_ERROR "LIBOQS_USE_SUBMODULE=ON but OQS_DIR is empty. Set -DOQS_DIR=/path/to/liboqs")
        endif()
        message(STATUS "Using liboqs from local directory: ${OQS_DIR}")
        add_subdirectory(${OQS_DIR} third_party/liboqs_build EXCLUDE_FROM_ALL)
        # Normalize target name to oqs::liboqs if liboqs created 'oqs' or 'oqs::oqs'
        if(TARGET oqs)
            add_library(oqs::liboqs ALIAS oqs)
        elseif(TARGET oqs::oqs)
            add_library(oqs::liboqs ALIAS oqs::oqs)
        else()
            message(FATAL_ERROR "liboqs added via add_subdirectory, but expected CMake target 'oqs' not found.")
        endif()
    else()
        # Prefer system liboqs if FindOQS succeeds
        if(OQS_FOUND)
            message(STATUS "Found system liboqs (via FindOQS). Using installed liboqs.")
            add_library(oqs::liboqs UNKNOWN IMPORTED)
            set_target_properties(oqs::liboqs PROPERTIES
                    IMPORTED_LOCATION "${OQS_LIBRARIES}"
                    INTERFACE_INCLUDE_DIRECTORIES "${OQS_INCLUDE_DIRS}")
        else()
            # FetchContent fallback
            include(FetchContent)
            message(STATUS "Fetching liboqs (tag ${LIBOQS_GIT_TAG}) via FetchContent...")
            FetchContent_Declare(
                    liboqs
                    GIT_REPOSITORY https://github.com/open-quantum-safe/liboqs.git
                    GIT_TAG ${LIBOQS_GIT_TAG}
            )
            FetchContent_GetProperties(liboqs)
            if(NOT liboqs_POPULATED)
                FetchContent_Populate(liboqs)
                add_subdirectory(${liboqs_SOURCE_DIR} ${liboqs_BINARY_DIR} EXCLUDE_FROM_ALL)
            endif()
            if(TARGET oqs)
                add_library(oqs::liboqs ALIAS oqs)
            elseif(TARGET oqs::oqs)
                add_library(oqs::liboqs ALIAS oqs::oqs)
            else()
                message(FATAL_ERROR "liboqs fetched but expected CMake target 'oqs' not found. Check liboqs CMake changes.")
            endif()
        endif()
    endif()
    add_compile_definitions(ENABLE_LIBOQS)
else()
    message(STATUS "PQC disabled (ENABLE_LIBOQS=OFF). Building without liboqs.")
endif()

# Hybrid TLS via OQS-OpenSSL
if(USE_OQS_OPENSSL)
    if(NOT OQS_OPENSSL_DIR)
        message(FATAL_ERROR "USE_OQS_OPENSSL=ON but OQS_OPENSSL_DIR is not set. Provide path to OQS-OpenSSL artifact.")
    endif()
    find_library(OQS_OPENSSL_SSL NAMES ssl libssl PATHS ${OQS_OPENSSL_DIR}/lib ${OQS_OPENSSL_DIR}/lib64 NO_DEFAULT_PATH)
    find_library(OQS_OPENSSL_CRYPTO NAMES crypto libcrypto PATHS ${OQS_OPENSSL_DIR}/lib ${OQS_OPENSSL_DIR}/lib64 NO_DEFAULT_PATH)
    if(NOT OQS_OPENSSL_SSL OR NOT OQS_OPENSSL_CRYPTO)
        message(FATAL_ERROR "Could not locate OQS-OpenSSL libraries under ${OQS_OPENSSL_DIR}/lib .")
    endif()
    add_library(oqs_openssl::lib UNKNOWN IMPORTED)
    set_target_properties(oqs_openssl::lib PROPERTIES
            IMPORTED_LOCATION "${OQS_OPENSSL_DIR}/lib/liboqs_openssl.a"
            INTERFACE_INCLUDE_DIRECTORIES "${OQS_OPENSSL_DIR}/include")
    add_compile_definitions(USE_OQS_OPENSSL)
endif()

# Subdirectories
add_subdirectory(src)


# Tests
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
