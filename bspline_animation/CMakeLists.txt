cmake_minimum_required(VERSION 3.7.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rg_lab1 VERSION 0.1.0)

set(MSYS_INCLUDES $ENV{MINGW_64}/include)
set(MSYS_LIB      $ENV{MINGW_64}/lib)
set(VK_SDK_INC    $ENV{VULKAN_SDK}/Include)
set(VK_SDK_LIB    $ENV{VULKAN_SDK}/Lib)
set(ASSETS assets)
set(SHADERS ${ASSETS}/shaders)
set(GLSL_COMPILER glslc)


add_executable(${PROJECT_NAME} app.h
                               app.cpp
                               constants.h
                               vertex.h
                               queue_families.h
                               queue_families.cpp
                               uniform.h
                               textures.h
                               textures.cpp
                               formats.h
                               models.h
                               bspline.h
                               bspline.cpp
                               main.cpp)

# Iz nekog razloga prestal radit find_package kak se spada

# find_package(Vulkan REQUIRED)

# if(NOT ${Vulkan_FOUND})
#     message(FATAL_ERROR "Vulkan not found!")
# else()
#     message(STATUS "Vulkan Found")
# endif()

# find_package(glfw3 3.3 REQUIRED)
# target_link_libraries(${PROJECT_NAME} glfw3)


target_include_directories(${PROJECT_NAME} PUBLIC ${VK_SDK_INC} ${MSYS_INCLUDES})
target_link_directories(${PROJECT_NAME} PUBLIC ${VK_SDK_LIB} ${MSYS_LIB})
target_link_libraries(${PROJECT_NAME} vulkan-1 glfw3)


file(GLOB_RECURSE GLSL_SOURCES ${SHADERS}/*.frag ${SHADERS}/*.vert ${SHADERS}/*.comp ${SHADERS}/*.geom)

foreach(GLSL_SOURCE_PATH ${GLSL_SOURCES})
    get_filename_component(GLSL_SOURCE_FILE ${GLSL_SOURCE_PATH} NAME)
    set(SPIRV_BIN ${CMAKE_BINARY_DIR}/shaders/${GLSL_SOURCE_FILE}.spv)
    add_custom_command(
        OUTPUT ${SPIRV_BIN}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders/
        COMMAND ${GLSL_COMPILER} ${GLSL_SOURCE_PATH} -o ${SPIRV_BIN}
        DEPENDS ${GLSL_SOURCE_PATH}
    )
    list(APPEND SPIRV_BINARIES ${SPIRV_BIN})
endforeach(GLSL_SOURCE_PATH)

add_custom_target(shaders
                  DEPENDS ${SPIRV_BINARIES})

add_dependencies(${PROJECT_NAME} shaders)


# target_include_directories(${PROJECT_NAME} PUBLIC ${MSYS_INCLUDES})