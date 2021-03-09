#include "src/example.h"
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/pixel_coders/RGB888_ARRAY.h>
#include <microgl/pixel_coders/RGB555_PACKED_16.h>
#include <microgl/pixel_coders/coder_converter.h>
#include <microgl/pixel_coders/coder_converter_rgba.h>

using namespace microgl;
using namespace microgl::color;
using namespace microgl::coder;

bool color_equals_rgb(color_t c1, color_t c2) {
    return c1.r==c2.r && c1.g==c2.g && c1.b==c2.b;
}

bool color_equals_rgba(color_t c1, color_t c2) {
    return color_equals_rgb(c1, c2) && c1.a==c2.a;
}

template<uint8_t r, uint8_t g, uint8_t b, uint8_t a>
color_t infer_max_color() {
    return {(1<<r)-1, (1<<g)-1, (1<<b)-1, (1<<a)-1};
}

template<typename Coder, uint8_t r, uint8_t g, uint8_t b, uint8_t a>
void test_coder_converter_rgba() {
    using coder_conv = coder::coder_converter_rgba<Coder, rgba_t<r,g,b,a>>;
    const color_t color = infer_max_color<r,g,b,a>();
    coder_conv coder;
    color_t result;
    typename coder_conv::pixel output;
    coder.encode(color, output);
    coder.decode(output, result);
    assertm(color_equals_rgb(color, result), "failed");
}

template<typename Coder1, typename Coder2>
void test_coder_converter() {
    using coder_conv = coder::coder_converter<Coder1, Coder2>;
    const color_t color = infer_max_color<Coder2::rgba::r,Coder2::rgba::g,Coder2::rgba::b,Coder2::rgba::a>();
    coder_conv coder;
    color_t result;
    typename coder_conv::pixel output;
    coder.encode(color, output);
    coder.decode(output, result);
    assertm(color_equals_rgb(color, result), "failed");
}


int main() {

    test_coder_converter_rgba<coder::RGB888_ARRAY, 5,6,5,0>();
    test_coder_converter_rgba<coder::RGB888_ARRAY, 3,2,3,0>();

    test_coder_converter<coder::RGB888_ARRAY, coder::RGB555_PACKED_16>();
    test_coder_converter<coder::RGB888_ARRAY, coder::RGB888_ARRAY>();
    test_coder_converter<coder::RGB888_ARRAY, coder::RGB888_PACKED_32>();

    std::cout << "all tests have passed" << std::endl;
    return 0;
}
