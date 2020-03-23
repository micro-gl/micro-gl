cmake_minimum_required(VERSION 3.14)
project(microgl)
enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
#endif()

message(${CMAKE_BUILD_TYPE})
#use this - should warn on inline functions that were not inlined -Winline
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g ")
set(CMAKE_CXX_FLAGS_RELEASE "-fno-exceptions -fstrict-aliasing -O3 -s -fno-rtti -flto -Wstrict-aliasing -fno-math-errno -ffast-math -fno-threadsafe-statics -Wno-unused-parameter")
set(sources
        include/public/microgl/Canvas.h
        src/PorterDuff.cpp
        src/PixelCoder.cpp
        src/color.cpp
        src/triangles.cpp
        src/qsort.cpp)

add_library(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
# expose public include directory
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/public
        ${PROJECT_SOURCE_DIR}/include/private
        )

# private include directory
#target_include_directories(${PROJECT_NAME}
#        PRIVATE
#        ${PROJECT_SOURCE_DIR}/src
#        .
#        ${PROJECT_SOURCE_DIR}/include/public
#        ${PROJECT_SOURCE_DIR}/include/public/microgl
#        ${PROJECT_SOURCE_DIR}/include/private/microgl
#        ${PROJECT_SOURCE_DIR}/include/private
#        )

#include_directories(${PROJECT_SOURCE_DIR}/include)

