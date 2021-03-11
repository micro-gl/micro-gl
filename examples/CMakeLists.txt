cmake_minimum_required(VERSION 3.12)
project(microgl-sdl-examples)
message(\n===========\n${PROJECT_NAME} \n===========\n)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG  ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE " -fno-rtti -fno-exceptions -fno-unwind-tables \
                            -fstrict-aliasing -O3 -flto \
                            -Wstrict-aliasing -fno-math-errno \
                            -ffast-math -fno-threadsafe-statics \
                            -fdata-sections -ffunction-sections \
                            " CACHE INTERNAL "" FORCE)
#-nostdlib
message("using compiler: ${CMAKE_CXX_COMPILER_ID}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)

if(SDL2_FOUND)
    set(libs ${SDL2_LIBRARY} microgl)

    message(${SDL2_FOUND})
    message(${SDL2_INCLUDE_DIR})
    message(${CMAKE_MODULE_PATH})
    message(${PROJECT_SOURCE_DIR})

    set(SOURCES
            example_blocks_3d_raster.cpp
            example_blocks_rounded_rect.cpp
            example_blocks_masks.cpp
            example_blocks_rect.cpp
            example_blocks_patch.cpp
            example_blend_modes.cpp
            example_draw_triangle.cpp
            example_draw_quadrilaterals.cpp
            example_draw_polygon.cpp
            example_draw_bezier_patch.cpp
            example_draw_rounded_rectangle.cpp
            example_draw_circles.cpp
            example_draw_masks.cpp
            example_clear.cpp
            example_sampler_texture_wrap.cpp
            example_sampler_texture.cpp
            example_sampler_gradient_radial.cpp
            example_sampler_gradient_line.cpp
            example_sampler_gradient_angular_linear.cpp
            example_sampler_gradient_axial.cpp
            example_sampler_gradient_2_colors_fast.cpp
            example_sampler_flat_color.cpp
            example_draw_triangles_matrix_transforms.cpp
            example_3d_single_triangle_shaders_with_ortho.cpp
            example_3d_shaders_flat_color.cpp
            example_3d_sampler_shader_and_z_buffer.cpp
            example_3d_simple_pipeline.cpp
            example_bentley_ottmann.cpp
            example_geometry_ear_clipping.cpp
            example_geometry_monotone_triangulation.cpp
            example_geometry_fan_triangulation.cpp
            example_geometry_complex_to_planar_subdivision.cpp
            example_geometry_stroke_tessellation.cpp
            example_geometry_stroke_dash_tessellation.cpp
            example_geometry_elliptic_arc_divider.cpp
            example_geometry_path.cpp
            example_q_sort.cpp
            example_avl_tree.cpp
            example_draw_bitmap_fonts.cpp
            example_draw_bitmap_packed.cpp
            example_draw_bitmap_indexed_palette.cpp
            example_lut.cpp
            example_draw_transparent.cpp
            example_draw_rect_axial.cpp
            example_draw_rect_transform.cpp
            example_3d_shader_color.cpp
            example_geometry_bezier_curve_divider.cpp
            test_coder_converter.cpp
            example_sampler_masking_sampler.cpp
            example_compositing.cpp
            example_mem.cpp
            test_coder_generator.cpp
            )

    set(SOURCES_SHARED
            libs/stb_image/stb_image.cpp
            src/Resources.cpp
            src/example.cpp src/example.h
            test.cpp)

    foreach( testsourcefile ${SOURCES} )
        # I used a simple string replace, to cut off .cpp.
        string( REPLACE ".cpp" "" testname ${testsourcefile} )
        include_directories("${SDL2_INCLUDE_DIR}")
        include_directories("${PROJECT_SOURCE_DIR}/src")
        include_directories("${PROJECT_SOURCE_DIR}/libs/stb_image")
        include_directories("${PROJECT_BINARY_DIR}")
        add_executable( ${testname} ${testsourcefile} ${SOURCES_SHARED} )
        # Make sure YourLib is linked to each app
        target_link_libraries( ${testname} ${libs} )
        add_custom_command(TARGET ${testname} POST_BUILD COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${testname}>/assets)
    endforeach( testsourcefile ${SOURCES} )

endif()

