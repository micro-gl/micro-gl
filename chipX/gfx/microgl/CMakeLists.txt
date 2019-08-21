cmake_minimum_required(VERSION 3.14)
project(microgl)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Release)
#endif()

message(${CMAKE_BUILD_TYPE})
#use this - should warn on inline functions that were not inlined -Winline
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-fstrict-aliasing -O3 -Wstrict-aliasing")

set(sources
        src/FrameBuffer.tpp
        include/public/microgl/FrameBuffer.h
        src/Bitmap.tpp
        include/public/microgl/Bitmap.h
        src/Buffer.tpp
        include/public/microgl/Buffer.h
        src/Canvas.tpp
        include/public/microgl/Canvas.h
        src/Functions.cpp
        src/PixelFormat.cpp
        src/BlendMode.cpp
        src/PorterDuff.cpp
        include/public/microgl/Q.h
        include/public/microgl/Sampler.h
        src/Curves.cpp
        include/private/microgl/EarClippingTriangulation.h
        include/private/microgl/BentleyOttmannLineIntersection.h
        include/private/microgl/AVLTree.h
        src/BentleyOttmannLineIntersection.cpp
        src/Rational.cpp
        src/Segment.cpp
        include/public/microgl/TriangleIndices.h)

add_library(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# expose public include directory
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/public
        ${PROJECT_SOURCE_DIR}/include/private
        )

# private include directory
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        .
        ${PROJECT_SOURCE_DIR}/include/public
        ${PROJECT_SOURCE_DIR}/include/public/microgl
        ${PROJECT_SOURCE_DIR}/include/private/microgl
        ${PROJECT_SOURCE_DIR}/include/private
        )

include_directories(${PROJECT_SOURCE_DIR}/include)

