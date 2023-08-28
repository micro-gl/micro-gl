# This file must be included by add_subdirectory() from parent, it doesn't work as standalone
cmake_minimum_required(VERSION 3.12)
project(microgl-sdl-examples)
message(\n===========\n${PROJECT_NAME} \n===========\n)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG  ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE " -fno-rtti -fno-exceptions -fno-unwind-tables \
                            -fstrict-aliasing -O3  -flto \
                            -Wstrict-aliasing -fno-math-errno \
                            -ffast-math -fno-threadsafe-statics \
                            -fdata-sections -ffunction-sections \
                            " CACHE INTERNAL "" FORCE)
#-nostdlib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(SDL2)

if(DEFINED SDL2_FOUND)
    set(libs ${SDL2_LIBRARY} microgl)
    set(SOURCES
            example_blocks_3d_raster.cpp
            example_blocks_rounded_rect.cpp
            example_blocks_rect.cpp
            example_blocks_patch.cpp
            example_blend_modes.cpp
            example_draw_triangle.cpp
            example_draw_quadrilaterals.cpp
            example_draw_polygon.cpp
            example_draw_bezier_patch.cpp
            example_draw_rounded_rectangle.cpp
            example_draw_circles.cpp
            example_draw_arcs.cpp
            example_draw_pie.cpp
            example_draw_masks.cpp
            example_draw_path.cpp
            example_draw_path_stroke.cpp
            example_clear.cpp
            example_sampler_texture_wrap.cpp
            example_sampler_texture.cpp
            example_sampler_gradient_radial.cpp
            example_sampler_gradient_line.cpp
            example_sampler_gradient_angular_linear.cpp
            example_sampler_gradient_axial.cpp
            example_sampler_gradient_2_colors_fast.cpp
            example_sampler_flat_color.cpp
            example_sampler_checkerboard.cpp
            example_sampler_void_sampler.cpp
            example_sampler_lines.cpp
            example_sampler_capsule.cpp
            example_sampler_circle.cpp
            example_sampler_linear_classifier.cpp
            example_sampler_1d_function.cpp
            example_sampler_quantize.cpp
            example_sampler_rounded_rect.cpp
            example_sampler_rect.cpp
            example_sampler_white_noise.cpp
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
            example_draw_bitmap_fonts.cpp
            example_draw_bitmap_packed.cpp
            example_draw_bitmap_indexed_palette.cpp
            example_draw_transparent.cpp
            example_draw_rect_axial.cpp
            example_draw_rect_transform.cpp
            example_3d_shader_color.cpp
            example_geometry_bezier_curve_divider.cpp
            example_sampler_masking_sampler.cpp
            example_compositing.cpp
            mtest_coder_converter.cpp
            mtest_coder_generator.cpp
            mtest_lut.cpp
            mtest_Q.cpp
    )

    set(SOURCES_SHARED
            libs/stb_image/stb_image.cpp
            src/Resources.cpp
            src/example.cpp src/example.h
            src/dummy.cpp src/dummy.h)

    foreach( testsourcefile ${SOURCES} )
        # I used a simple string replace, to cut off .cpp.
        string( REPLACE ".cpp" "" testname ${testsourcefile} )
        include_directories("${SDL2_INCLUDE_DIR}")
        include_directories("${PROJECT_SOURCE_DIR}/src")
        include_directories("${PROJECT_SOURCE_DIR}/libs/stb_image")
        include_directories("${PROJECT_BINARY_DIR}")

        add_executable( ${testname} ${testsourcefile} ${SOURCES_SHARED} )
        #        add_executable( ${testname} ${testsourcefile}  )
        # Make sure YourLib is linked to each app
        target_link_libraries( ${testname} ${libs} )
        # this is redundant, as target file dir is always delegated to CMAKE_RUNTIME_OUTPUT_DIRECTORY
        add_custom_command(TARGET ${testname} POST_BUILD COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${testname}>/assets)
    endforeach( testsourcefile ${SOURCES} )

endif()

# newer clion does not make the binary executable location the current working dir,
# so I copy the assets also to the top level build directory.
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

