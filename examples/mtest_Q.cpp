#include <microgl/Q.h>
#include <microgl/math/q_math.h>
#include <iostream>

int main() {
    std::cout <<std::to_string(_LIBCPP_STD_VER) << std::endl;

    using q = Q<4, int32_t>;

    q a{9};
    auto b = int(microgl::math::sqrt(a));
    auto c = microgl::math::sqrt(a);
}

