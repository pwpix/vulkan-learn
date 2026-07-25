# Findglfw3.cmake
#
# Finds the GLFW3 library
#
# This will define the following variables:
#
#    glfw3_FOUND
#    GLFW3_INCLUDE_DIR
#    GLFW3_LIBRARY
#
# and the following imported targets:
#
#    glfw
#

# Try to find GLFW3 using CONFIG mode first (common for vcpkg)
find_package(glfw3 CONFIG QUIET)

if (glfw3_FOUND)
    if (NOT TARGET glfw)
        if (TARGET glfw3::glfw)
            add_library(glfw ALIAS glfw3::glfw)
        endif ()
    endif ()

    # Set variables for find_package_handle_standard_args
    if (NOT GLFW3_INCLUDE_DIR)
        get_target_property(GLFW3_INCLUDE_DIR glfw INTERFACE_INCLUDE_DIRECTORIES)
    endif ()
    if (NOT GLFW3_LIBRARY)
        set(GLFW3_LIBRARY glfw)
    endif ()
endif ()

if (NOT glfw3_FOUND)
    # Try to find using standard find_path/find_library
    find_path(GLFW3_INCLUDE_DIR NAMES GLFW/glfw3.h)
    find_library(GLFW3_LIBRARY NAMES glfw glfw3)

    if (GLFW3_INCLUDE_DIR AND GLFW3_LIBRARY)
        set(glfw3_FOUND TRUE)
        if (NOT TARGET glfw)
            add_library(glfw UNKNOWN IMPORTED)
            set_target_properties(glfw PROPERTIES
                    IMPORTED_LOCATION "${GLFW3_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${GLFW3_INCLUDE_DIR}"
            )
        endif ()
    endif ()
endif ()

if (NOT glfw3_FOUND)
    
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glfw3
        REQUIRED_VARS GLFW3_INCLUDE_DIR GLFW3_LIBRARY
)
