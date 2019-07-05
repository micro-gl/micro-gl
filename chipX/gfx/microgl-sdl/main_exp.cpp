//
<<<<<<< HEAD
// Created by Tomer Shalev on 2019-06-15.
// this is a sandbox for playing with microgl lib
=======
// Created by Tomer Shalev on 2019-06-29.
>>>>>>> new_arch
//

#include <stdio.h>
#include <iostream>
<<<<<<< HEAD
#include <chrono>

class TestClass {
public:
    template<typename T>
    explicit TestClass(const T &t) {
        std::cout << "Constructed a TestClass " << t << std::endl;
    }

};

#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Any {
public:

    // r-value reference copy constructor
    template <typename T>
    Any(T&& value)
            : content_(make_unique<Holder<T>>(std::forward<T>(value))) {
        std::cout << "";
    }

    // l-value copy constructor
    template <typename T>
    Any(T& value)
            : content_(make_unique<Holder<T>>(value)) {
        std::cout << "";
    }

    Any& operator=(Any other) {
//        swap(other);

        return *this;
    }

    template <typename T>
    T& get() {
//        if (std::type_index(typeid(T)) == std::type_index(content_->type_info())) {
            return static_cast<Holder<T>&>(*content_).value_;
//        }
        throw std::bad_cast();
    }

private:
    struct Placeholder {
        virtual ~Placeholder() = default;
        virtual const std::type_info& type_info() const = 0;
        virtual std::unique_ptr<Placeholder> clone() = 0;
    };

    template <typename T>
    struct Holder : public Placeholder {
        template <typename U>
        explicit Holder(U&& value) : value_(std::forward<U>(value)) {}
        const std::type_info& type_info() const override { return typeid(T); }
        std::unique_ptr<Placeholder> clone() override {
            return make_unique<Holder<T>>(value_);
        }
        T value_;
    };

    std::unique_ptr<Placeholder> content_;
};

int main() {
    Any a = 1;
    Any a2 = 10;
    a = std::string("sdsds");

    int x = 5;

    a = a2;

    std::cout << a.get<int>();

}
=======

#include <string>
using namespace std;

 struct vec {
     uint8_t x;
     uint8_t y;
     uint8_t z;
}  ;

//typedef uint32_t RGB[3]  __attribute__((aligned(8)));
int N = 640*480*1;

uint32_t * buf_1 = new uint32_t[N] ;
vec * buf_2 = new vec[N];


void render_1() noexcept {

    for (int jx = 0; jx < 1000; ++jx) {

        for (int ix = 0; ix < N; ++ix) {
            buf_1[ix] =  5;
//            buf_2[ix].x =  ix;
//            buf_2[ix].y =  ix;
//            buf_2[ix].z =  ix;
        }

    }

}

int render_test(int M) {
    auto ms = std::chrono::milliseconds(1);
    auto start = std::chrono::high_resolution_clock::now();

//    for (int i = 0; i < M; ++i) {
        render_1();
//    }

    auto end = std::chrono::high_resolution_clock::now();

    return (end-start)/(ms*M);
}



int main() {

    int ms = render_test(1);

    std::cout << ms<<std::endl;



    return 0;

}
>>>>>>> new_arch
