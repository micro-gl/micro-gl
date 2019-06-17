cmake_minimum_required(VERSION 3.14)
project(microgl)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

set(sources
        src/FrameBuffer.tpp
        include/FrameBuffer.h
        src/Canvas.tpp
        include/Canvas.h
        src/Functions.cpp
        src/PixelFormat.cpp
        src/BlendMode.cpp
        src/PorterDuff.cpp)

add_library(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# expose public include directory
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

# private include directory
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src . ${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/include)

