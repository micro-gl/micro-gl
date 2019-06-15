cmake_minimum_required(VERSION 3.14)
project(chipxengine)

set (CMAKE_CXX_STANDARD 11)
set(sources
    src/BaseAudioEngine.cpp
    src/Logger.cpp)

add_library(${PROJECT_NAME} ${sources})

# expose public include directory
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

# private include directory
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src .)