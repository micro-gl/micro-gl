#include <iostream>
#include <microgl/math.h>

using namespace microgl;

namespace microgl {
    template<>
    void foo<float>(float val) {
        val = 1;
    }

    template<unsigned N>
    void foo(Q<N> val) {
        val = 1;
    }

}

int main() {
    microgl::foo<float>(5.0f);
    microgl::foo<int>(5.0f);
    microgl::foo(Q<4>{5.0f});

    std::cout <<std::to_string(_LIBCPP_STD_VER) << std::endl;
}

