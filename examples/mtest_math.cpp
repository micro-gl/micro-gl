#include "microgl/math/Q.h"

namespace microgl {
//    template<>
//    void foo<float>(float val) {
//        val = 1;
//    }

    template<unsigned N>
    void foo3(Q<N> val) {
        val = 1;
    }
//
//    void foo3(Q<4> val) {
//        val = 1;
//    }
//
//    template<>
//    void foo2(Q<4> val) {
//        val = 1;
//    }
//
//    template<>
//    void foo2(float val) {
//        val = 1;
//    }

//    void foo3(float val) {
//        val = 1;
//    }
    void foo3(float val) {
        val =0;
    }


//    template<>
//    void foo2(int val) {
//        val = 1;
//    }


}

#include <iostream>
#include <microgl/math.h>

//using namespace microgl;

int main() {
    microgl::do2<float>(5.0f);
    microgl::do2<float>(5.0f);
    microgl::do2<int>(5.0f);
    microgl::do2(Q<14>{5.0f});

    std::cout <<std::to_string(_LIBCPP_STD_VER) << std::endl;
}

