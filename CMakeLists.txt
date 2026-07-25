cmake_minimum_required(VERSION 3.29)

project(sally)

# Add option to enable/disable c++20 module
option(ENABLE_CPP20_MODULE "Enable C++ 20 module support for Vulkan" OFF)

#Enable C++ module dependency scanning only if C++ 20 module is enabled
if(ENABLE_CPP20_MODULE)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/external/Slang")
#find_package(slang REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)
find_package(Vulkan 1.4.335 REQUIRED)
find_package(KTX CONFIG REQUIRED)
find_package(GLFW3 CONFIG REQUIRED)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)

include(${CMAKE_MODULE_PATH}/dependencies.cmake)

# Create a dummy interface library when c++ 20 module is disabled
# Modules are disabled in this project
add_library(VulkanCppModule INTERFACE)
add_library(Vulkan::cppm ALIAS VulkanCppModule)
target_link_libraries(VulkanCppModule INTERFACE Vulkan::Vulkan)
target_compile_definitions(VulkanCppModule
    INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)
#end of dummy interface


file(GLOB_RECURSE v0_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/v0/window/*.cpp"
)

add_library(window STATIC 
    ${v0_SOURCE_FILES}
)
target_include_directories(window PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/v0/window
)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/v0/main.cpp")
set(STB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/v0/stb_image.cpp")

add_executable(test ${SOURCES} ${v0_SOURCE_FILES})


target_sources(test PRIVATE ${STB_SOURCES})
target_link_libraries(test PRIVATE
    window
    stb
    Vulkan::Vulkan
    glfw
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
target_compile_options(test PRIVATE -fsanitize=address)
target_link_options(test PRIVATE -fsanitize=address)
