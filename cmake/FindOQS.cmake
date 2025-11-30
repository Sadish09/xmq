# IMPORTANT: Change path if you have it installed in a wierd path, add regex if you speak wizard

# Set:
#   OQS_FOUND (BOOL)
#   OQS_INCLUDE_DIRS
#   OQS_LIBRARIES
# And create imported target oqs::liboqs on success.

if(DEFINED OQS_FOUND)
    return()  # already run
endif()

find_path(OQS_INCLUDE_DIR NAMES oqs/oqs.h PATHS /usr/include /usr/local/include NO_DEFAULT_PATH)
find_library(OQS_LIBRARY NAMES oqs oqs_shared oqs_static oqs_lib PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64 NO_DEFAULT_PATH)

if(OQS_INCLUDE_DIR AND OQS_LIBRARY)
    set(OQS_FOUND TRUE CACHE BOOL "liboqs found" FORCE)
    set(OQS_INCLUDE_DIRS ${OQS_INCLUDE_DIR} CACHE PATH "liboqs include dir")
    set(OQS_LIBRARIES ${OQS_LIBRARY} CACHE PATH "liboqs library")

    # Create an imported target for convenience
    if(NOT TARGET oqs::liboqs)
        add_library(oqs::liboqs UNKNOWN IMPORTED)
        set_target_properties(oqs::liboqs PROPERTIES
                IMPORTED_LOCATION "${OQS_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${OQS_INCLUDE_DIRS}")
    endif()

    message(STATUS "FindOQS: found liboqs include=${OQS_INCLUDE_DIRS} lib=${OQS_LIBRARIES}")
else()
    set(OQS_FOUND FALSE CACHE BOOL "liboqs found" FORCE)
    message(STATUS "FindOQS: liboqs not found on system (this is OK if using FetchContent).")
endif()
