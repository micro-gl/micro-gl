#include <iostream>
//#include <vector>
#include <cstdio>
#include <cstdlib>
#include <microgl/lut_bits.h>
#include <new>
#include <vector>
#include <memory>
#include <type_traits>
using size_t = unsigned long;

// Default placement versions of operator new.
//inline void* operator new(size_t, void* __p) noexcept { return __p; }
//inline void* operator new[](size_t, void* __p) { return __p; }
//
// Default placement versions of operator delete.
//inline void  operator delete  (void*, void*) throw() { }
//inline void  operator delete[](void*, void*) throw() { }
void * mem;
void * operator new[](size_t size)
{
    std::cout << "New [] operator overloading " << std::endl;
    return mem;
    void * p = (void *)std::malloc(size);
    return p;
}

void * operator new(size_t size)
{
    std::cout << "New operator overloading " << std::endl;
    return mem;
}
//
//void operator delete(void *p) noexcept
//{
////    free(p);
//}
//
//void* operator new[](size_t size)
//{
//    return operator new(size); // Same as regular new
//}
//
void operator delete[](void *p) noexcept
{
    std::cout << "delete [] operator overloading " << std::endl;
    operator delete(p); // Same as regular delete
}

//void* operator new(size_t size, std::nothrow_t) noexcept
//{
//    return operator new(size); // Same as regular new
//}
//
//void operator delete(void *p,  std::nothrow_t) noexcept
//{
//    operator delete(p); // Same as regular delete
//}
//
//void* operator new[](size_t size, std::nothrow_t) noexcept
//{
//    return operator new(size); // Same as regular new
//}
//
//void operator delete[](void *p,  std::nothrow_t) noexcept
//{
//    operator delete(p); // Same as regular delete
//}

struct A {
    int a;
    int b;
    int c;
    A(int _a=5, int _b=6) : a{_a}, b{_b} {
        std::cout << "CC" << std::endl;
    }

    ~A() {
        std::cout << "DD" << std::endl;
    }
};

template<typename A, typename B>
struct GGG {
    GGG(A a) {
        a.~A();
    }
};

template<typename A, template<typename...> class cont>
struct GGG2 {
    using con = cont<A, A>;
    con aa;

    GGG2(con aa) : aa{aa} {

    };
};

int main() {
    std::cout <<  std::endl << sizeof (long) << std::endl;
    GGG<int, void> s(5);
std::malloc
    uint8_t mem_[sizeof (A)*255]{};
    mem = mem_;
    std::cout <<std::to_string(_LIBCPP_STD_VER) << std::endl;
//    auto * mem = new uint8_t [sizeof (A)*3];
//    A * p = new(mem) A();
//    auto * mem2= new(mem) A[3];
    auto * mem21= new A;
    auto * mem2= new A[255];
//    auto * mem3 = ((void *)mem2) A(7,8);
//    ::operator delete[](mem2);
    delete [] mem2;
//    std::cout << mem2->a;
//    std::cout << p->a;
    return 0;
}

