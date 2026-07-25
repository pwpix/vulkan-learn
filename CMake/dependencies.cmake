message(STATUS "Checking dependencies")

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

#SLANG
#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../external/Slang")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../external/Slang") #directory of the current cmake module

find_package(slang REQUIRED)

#TINYGLTF
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/../external/tinygltf")
find_package(TinyGLTF CONFIG REQUIRED)

#set_target_properties(tinygltf PROPERTIES LINKER_LANGUAGE CXX)

#STB
set(STB_INCLUDE_DIR "${EXTERNAL_DIR}/stb")
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${STB_INCLUDE_DIR}
)

