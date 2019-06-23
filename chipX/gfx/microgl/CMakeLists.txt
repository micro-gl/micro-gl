cmake_minimum_required(VERSION 3.14)
project(microgl)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)
#if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
#endif()

message(${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(sources
        src/FrameBuffer.tpp
        include/microgl/FrameBuffer.h
        src/Bitmap.tpp
        include/microgl/Bitmap.h
        src/Buffer.tpp
        include/microgl/Buffer.h
        src/Canvas.tpp
        include/microgl/Canvas.h
        src/Functions.cpp
        src/PixelFormat.cpp
        src/BlendMode.cpp
        src/PorterDuff.cpp
        )

add_library(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# expose public include directory
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

# private include directory
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src . ${PROJECT_SOURCE_DIR}/include/microgl)

include_directories(${PROJECT_SOURCE_DIR}/include)

