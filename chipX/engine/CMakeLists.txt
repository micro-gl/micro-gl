cmake_minimum_required(VERSION 3.14)
project(chipxengine)

include_directories(.)

set(sources
        src/BaseAudioEngine.cpp
        src/Logger.cpp)

add_library(chipxengine ${sources})

target_include_directories(chipxengine
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_include_directories(chipxengine
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src)