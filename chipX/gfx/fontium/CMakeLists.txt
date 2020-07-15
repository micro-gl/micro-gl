cmake_minimum_required(VERSION 3.12)
project(fontium)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-fno-rtti -fstrict-aliasing -O3 -s -flto -Wstrict-aliasing -fno-math-errno -ffast-math -fno-threadsafe-statics -Wno-unused-parameter")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/libs/lodepng")

set(SOURCES
        libs/lodepng/lodepng.cpp
        libs/stb_image/stb_image.cpp
        Imagium.cpp types.cpp bits.cpp utils.cpp)
set(TARGET_LIB ${PROJECT_NAME})
set(TARGET_CLI ${PROJECT_NAME}-cli)

add_executable(${TARGET_CLI} ${SOURCES} imagium-cli.cpp)
add_library(${TARGET_LIB} ${SOURCES})
add_custom_command(TARGET ${TARGET_CLI} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)