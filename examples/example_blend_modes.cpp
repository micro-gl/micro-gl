#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/porter_duff/FastSourceOverOnOpaque.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/blend_modes/Multiply.h>
#include <microgl/blend_modes/Overlay.h>
#include <microgl/blend_modes/PinLight.h>
#include <microgl/blend_modes/Screen.h>
#include <microgl/blend_modes/Subtract.h>
#include <microgl/blend_modes/Darken.h>
#include <microgl/blend_modes/Lighten.h>
#include <microgl/blend_modes/Difference.h>
#include <microgl/blend_modes/Divide.h>
#include <microgl/blend_modes/ColorBurn.h>
#include <microgl/blend_modes/ColorDodge.h>
#include <microgl/blend_modes/Exclusion.h>
#include <microgl/blend_modes/HardLight.h>
#include <microgl/blend_modes/VividLight.h>
#include <microgl/blend_modes/HardMix.h>
#include <microgl/blend_modes/LinearBurn.h>
#include <microgl/blend_modes/LinearDodge.h>
#include <microgl/blend_modes/LinearLight.h>
#include <microgl/blend_modes/SoftLight.h>
#include <microgl/samplers/flat_color.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 480*1

using namespace microgl::color;

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    sampling::flat_color<> color_grey{{128,128,128,255}};
    Resources resources{};

    Canvas24 canvas(W, H);

    auto render = [&]() -> void {
        canvas.clear({0, 255, 255, 255});
        canvas.drawRect<blendmode::Multiply<true>, porterduff::FastSourceOverOnOpaque>(
//        canvas.drawRect<blendmode::Normal, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Overlay<false>, porterduff::None<true>>(
//        canvas.drawRect<blendmode::PinLight, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Screen<false>, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Subtract, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Darken, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Lighten, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Difference, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Divide, porterduff::None<true>>(
//        canvas.drawRect<blendmode::ColorDodge, porterduff::None<true>>(
//        canvas.drawRect<blendmode::Exclusion<true>, porterduff::None<true>>(
//        canvas.drawRect<blendmode::HardLight<false, false>, porterduff::None<true>>(
//        canvas.drawRect<blendmode::VividLight, porterduff::None<true>>(
//        canvas.drawRect<blendmode::HardMix, porterduff::None<true>>(
//        canvas.drawRect<blendmode::LinearLight, porterduff::None<true>>(
//        canvas.drawRect<blendmode::SoftLight<true>, porterduff::None<true>>(
                color_grey,
                -0, -0, 300, 300,
                255);
    };

    example_run(&canvas, render);
}
