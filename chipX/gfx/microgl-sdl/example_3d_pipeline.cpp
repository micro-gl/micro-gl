#include <microgl/color.h>
#include <microgl/Canvas.h>
#include <microgl/matrix_4x4.h>
#include <microgl/camera.h>
#include <microgl/pipeline.h>
#include <microgl/dynamic_array.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include "data/model_3d_tree.h"
#include "data/model_3d_cube.h"
#include "src/example.h"

#define TEST_ITERATIONS 1
#define W 640*1
#define H 480*1

using Bitmap24= Bitmap<coder::RGB888_PACKED_32>;
using Canvas24= Canvas<Bitmap24>;

template <typename number>
using arr = dynamic_array<vec3<number>>;

template <typename number_coords, typename Canvas>
void render_template(const model_3d<number_coords> & object, Canvas & canvas,
                     float time) {
    using vertex = vec3<number_coords>;
    using camera = microgl::camera<number_coords>;
    using mat4 = matrix_4x4<number_coords>;
    using math = microgl::math;

    float z=time;

//    canvas->updateClipRect(W/2,50,600,600);
    int canvas_width = canvas.width();
    int canvas_height = canvas.height();

    mat4 model = mat4::transform({ 0, math::deg_to_rad(z),
                                   math::deg_to_rad(0/2)},
                                 {0,0,-300+z},
                                 {10,10,10});
//    mat4 view = camera::lookAt({0, 0, -z}, {0,0, -z-1}, {0,1,0});
    mat4 view = camera::lookAt({0, 0, 100},
                               {0,0, 0},
                               {0,1,0});
    mat4 projection = camera::perspective(math::deg_to_rad(60),
                                          canvas_width, canvas_height, 1, 500);
//    mat4 projection = camera::perspective(-1,1,-1,1,1,10000);
    mat4 mvp = projection * view * model;
    canvas.clear({255,255,255,255});
    microgl::_3d::pipeline<number_coords, decltype(canvas)>::render (
            {250,0,255,255},
            object.vertices.data(),
            object.vertices.size(),
            object.indices.data(),
            object.indices.size(),
            mvp,
            triangles::indices::TRIANGLES,
            canvas, 255
            );

}

int main() {
    auto * canvas = new Canvas24(W, H);

    struct data_t {
        float time = 0;
    } data;

    auto render = [&](data_t & d) -> void {
        d.time+=0.1121;
//        render_template<float>(tree_3d<float>, *canvas, d.time);
//        render_template<float>(cube_3d<float>, *canvas, d.time);
//        render_template<Q<10>>(tree_3d<Q<10>>, *canvas, d.time);
        render_template<Q<15>>(tree_3d<Q<15>>, *canvas, d.time);
    };

    example_run(canvas, data,
                TEST_ITERATIONS,
                render);
}
