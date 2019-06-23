cmake_minimum_required(VERSION 3.12)
project(microgl-sdl)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# I added FindSDL2.cmake module so it can find sdl2 in a platform agnostic manner
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
#find_package(OpenGL REQUIRED)

# link against these shared libraries
set(libs ${SDL2_LIBRARY})

#include SDL2 headers and OpenGL Headers
include_directories(${SDL2_INCLUDE_DIR})

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/libs/stb_image")
include_directories("${PROJECT_BINARY_DIR}")

set(SOURCES main.cpp
        libs/stb_image/stb_image.cpp
        src/Resources.cpp
        )

add_executable(${PROJECT_NAME}
               ${SOURCES}
                )

target_link_libraries(${PROJECT_NAME} ${libs} microgl)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)