#include <iostream>
#include <initializer_list>
#include <microgl/dynamic_array.h>

int main() {

    using container = dynamic_array<int>;
    using il = std::initializer_list<int>;

    container c {il{1,2,3,4,5}};
    container d {c};
    container e {4};
}

