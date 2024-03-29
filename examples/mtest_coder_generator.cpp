#include "src/example.h"
#include <microgl/pixel_coders/RGBA_PACKED.h>
#include <microgl/pixel_coders/RGBA_UNPACKED.h>

#define TEST_ITERATIONS 100
#define W 640*1
#define H 640*1

using index_t = unsigned int;
float t=0;
using namespace microgl;

bool color_equals(color_t c1, color_t c2) {
    return c1.r==c2.r && c1.g==c2.g && c1.b==c2.b &&c1.a==c2.a;
}

template<microgl::ints::uint8_t r, microgl::ints::uint8_t g, microgl::ints::uint8_t b, microgl::ints::uint8_t a>
color_t infer_max_color() {
    return {(1<<r)-1, (1<<g)-1, (1<<b)-1, (1<<a)-1};
}

template<microgl::ints::uint8_t r, microgl::ints::uint8_t g, microgl::ints::uint8_t b, microgl::ints::uint8_t a,
        microgl::ints::uint8_t ri=0, microgl::ints::uint8_t gi=1, microgl::ints::uint8_t bi=2, microgl::ints::uint8_t ai=3>
void test_packed_coder() {
    using Coder = coder::RGBA_PACKED<r,g,b,a, ri,gi,bi,ai>;
    Coder coder;
    color_t color_input= infer_max_color<r,g,b,a>(), color_output{0,0,0,0};
    typename Coder::pixel output;

    coder.encode(color_input, output);
    coder.decode(output, color_output);
    assertm(color_equals(color_input, color_output), "test_packed_coder failed");
}

template<microgl::ints::uint8_t r, microgl::ints::uint8_t g, microgl::ints::uint8_t b, microgl::ints::uint8_t a>
void test_unpacked_coder() {
    using Coder = coder::RGBA_UNPACKED<r,g,b,a>;
    Coder coder;
    color_t color_input= infer_max_color<r,g,b,a>(), color_output{0,0,0,0};
    typename Coder::pixel output;
    coder.encode(color_input, output);
    coder.decode(output, color_output);
    assertm(color_equals(color_input, color_output), "test_unpacked_coder failed");
}

int main() {
    test_packed_coder<5,6,5,8, 2,1,3,0>();
    test_packed_coder<5,6,5,8, 0,1,2,3>();
    test_packed_coder<2,3,2,1, 0,1,2,3>();
    test_packed_coder<2,3,2,1, 0,1,2,3>();

    test_unpacked_coder<5,6,5,8>();

    std::cout << "all tests have passed" << std::endl;
}
