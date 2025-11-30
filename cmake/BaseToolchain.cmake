# IMPORTANT: Edit this file for custom build options, basic ones are already provided
if(NOT DEFINED ENABLE_SANITIZERS)
    set(ENABLE_SANITIZERS OFF CACHE BOOL "Enable ASan/UBSan for debug builds")
endif()

# MSVC options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic) # Basic warnings and ISO Compliance
endif()

# Release / debug tuning
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(NOT MSVC)
        add_compile_options(-O3 -fno-omit-frame-pointer) # Compile time optimizations for RELEASE builds; perf & VTune supported, edit for custom profilers
    endif()
else()
    if(NOT MSVC)
        add_compile_options(-O0 -g) # Reduce headache for debug builds
    endif()
endif()

# Sanitizers (only for non-MSVC GNU/Clang)
if(ENABLE_SANITIZERS AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    message(STATUS "Enabling sanitizers: address, undefined")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Helper macro: add_arch_specific_compile_options(target)
macro(add_arch_specific_compile_options target)
    if(DEFINED ARCH_X86)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # Default to safe x86 baseline; allow users to override -march via cache var
            if(NOT DEFINED X86_MARCH)
                set(X86_MARCH "-march=x86-64" CACHE STRING "x86 march flag")
            endif()
            target_compile_options(${target} PRIVATE ${X86_MARCH})
        endif()
    elseif(DEFINED ARCH_ARM)
        if(NOT DEFINED ARM_MARCH)
            set(ARM_MARCH "-march=armv8-a" CACHE STRING "ARM march flag")
        endif()
        target_compile_options(${target} PRIVATE ${ARM_MARCH})
    endif()
endmacro()

# Helper macro: add_common_targets()
macro(add_common_library name sources)
    add_library(${name} ${sources})
    target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    add_arch_specific_compile_options(${name})
endmacro()
