cmake_minimum_required(VERSION 3.14)
project(microgl)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

message("===============\n${PROJECT_NAME} project \n===============")

set(sources
        src/PixelCoder.cpp
        src/triangles.cpp
        src/qsort.cpp)

add_library(${PROJECT_NAME} ${sources})
#set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
# expose public include directory
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/public
        )

add_subdirectory(examples)
