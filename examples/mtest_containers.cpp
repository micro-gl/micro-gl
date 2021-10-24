#include <iostream>
#include <initializer_list>
#include <vector>
#include <microgl/dynamic_array.h>

struct dummy {
    dummy() {
        std::cout << "Constructed" <<std::endl;

    }
    ~dummy() {
        std::cout << "destructed" <<std::endl;

    }
};
int main() {

    using container_t = std::vector<dummy>;
    container_t container{10};

//    container.push_back(dummy());
//    container.push_back(dummy());
//    container.push_back(dummy());
    container.clear();
    container.push_back(dummy());

}

