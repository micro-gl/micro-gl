#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include "microgl/math/matrix_4x4.h"
#include <microgl/camera.h>
#include <microgl/pipeline.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include "data/model_3d_tree.h"
#include "data/model_3d_cube.h"
#include "src/example.h"

#define W 640*1
#define H 480*1

int main() {
    using number = float;
//    using number = Q<10>;
//    using number = Q<15>;

    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    Canvas24 canvas(W, H);

    auto render_template = [&](const model_3d<number> & object) {
        using vertex = vertex3<number>;
        using camera = microgl::camera;
        using mat4 = matrix_4x4<number>;
        using namespace microgl::math;

        static float z=0.0f;

        z+=0.4f;

        int canvas_width = canvas.width();
        int canvas_height = canvas.height();

        mat4 model = mat4::transform({ 0, math::deg_to_rad(z),
                                       math::deg_to_rad(0.0f/2)},
                                     {0,0,-300+z},
                                     {10,10,10});
//        mat4 view = camera::lookAt<number>({0, 0, -z}, {0,0, -z-1}, {0,1,0});
        mat4 view = camera::lookAt<number>({0, 0, 100},
                                   {0,0, 0},
                                   {0,1,0});
        mat4 projection = camera::perspective<number>(math::deg_to_rad(60.0f),
                                              canvas_width, canvas_height, 1, 500);
        //mat4 projection = camera::perspective<number>(-1,1,-1,1,1,10000);
        mat4 mvp = projection * view * model;
        canvas.clear({255,255,255,255});
        microgl::_3d::pipeline<number, decltype(canvas)>::render (
                {250,0,255,255},
                object.vertices.data(),
                object.vertices.size(),
                object.indices.data(),
                object.indices.size(),
                mvp,
                microtess::triangles::indices::TRIANGLES,
                canvas, 255
        );

    };

    auto render = [&](void*, void*, void*){
        static auto model = tree_3d<number>;

        render_template(model);
    };

    example_run(&canvas, render);

    return 0;
}
