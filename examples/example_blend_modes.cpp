#include "src/Resources.h"
#include "src/example.h"
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
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

int main() {
    using Canvas24= canvas<bitmap<coder::RGB888_PACKED_32>>;
    sampling::flat_color<> color_grey{{128,128,128,255}};

    Canvas24 canvas(W, H);

    auto render = [&](void*, void*, void*) -> void {
        canvas.clear({0, 255, 255, 255});
        canvas.drawRect<blendmode::Multiply<true>, porterduff::FastSourceOverOnOpaque>(
//        canvas.drawRect_internal<blendmode::Normal, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Overlay<false>, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::PinLight, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Screen<false>, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Subtract, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Darken, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Lighten, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Difference, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Divide, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::ColorDodge, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::Exclusion<true>, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::HardLight<false, false>, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::VividLight, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::HardMix, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::LinearLight, porterduff::None<true>>(
//        canvas.drawRect_internal<blendmode::SoftLight<true>, porterduff::None<true>>(
                color_grey,
                -0, -0, 300, 300,
                255);
    };

    example_run(&canvas, render);

    return 0;
}
