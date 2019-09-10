cmake_minimum_required(VERSION 3.12)
project(microgl-sdl)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-fstrict-aliasing -O3 -Wstrict-aliasing ")
message("using compiler: ${CMAKE_CXX_COMPILER_ID}")
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

set(SOURCES
#        main_clear.cpp
#        main_curves.cpp
#        main_speed_optimization.cpp
#        main_quadrilaterals.cpp
#        main_canvas_draw_polygon.cpp
#        main_path_tessellation.cpp
#        main_arc_divider.cpp
#        main_fan_triangulation.cpp
#        main_ear_clipping.cpp
#        main_affine_matrix.cpp
        main_Q_matrix_transforms.cpp
#        main_matrix.cpp
#        main_aa_bug.cpp
#        main_aa_bug_2.cpp
#        main_bentley_ottmann.cpp
#        main_avl_tree.cpp
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