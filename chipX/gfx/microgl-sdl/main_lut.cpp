#include <iostream>
#include <microgl/lut_bits.h>

void test_static_lut() {
    std::cout << int(microgl::lut::static_lut_bits<5,8>::get(15)) << std::endl;
}

void test_dynamic_lut() {
    microgl::lut::dynamic_lut_bits<5, 8, true> lut;
    std::cout << int(lut[31]) << std::endl;
}

int main() {
    test_static_lut();
    test_dynamic_lut();

}

