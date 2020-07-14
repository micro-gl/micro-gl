cmake_minimum_required(VERSION 3.12)
project(imagium)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-fno-rtti -fno-exceptions -fstrict-aliasing -O3 -s -flto -Wstrict-aliasing -fno-math-errno -ffast-math -fno-threadsafe-statics -Wno-unused-parameter")
#set(CMAKE_C_COMPILER "gcc-4.2")
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")

message("using compiler: ${CMAKE_CXX_COMPILER_ID}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/libs/lodepng")
include_directories("${PROJECT_BINARY_DIR}")

set(SOURCES
        cli.cpp
        libs/lodepng/lodepng.cpp
        libs/stb_image/stb_image.cpp
        Imagium.cpp types.cpp bits.cpp utils.cpp)

add_executable(${PROJECT_NAME}
               ${SOURCES}
                )

target_link_libraries(${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)