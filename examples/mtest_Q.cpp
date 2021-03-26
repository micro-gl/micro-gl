#include <microgl/Q.h>
#include <iostream>

int main() {
    std::cout <<std::to_string(_LIBCPP_STD_VER) << std::endl;

    using q = Q<4>;

    q a{(unsigned int)(4)}, b{1}, c{2};

    q a1 = 7.0f;
    a1= a1 + 5.11f;
    a1/=5.11f;
}

