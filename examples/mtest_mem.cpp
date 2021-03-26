//#include <iostream>
//#include <vector>
#include <microgl/lut_bits.h>
//#include <new>
using size_t = unsigned long;

// Default placement versions of operator new.
inline void* operator new(size_t, void* __p)  { return __p; }
//inline void* operator new[](size_t, void* __p) { return __p; }
//
// Default placement versions of operator delete.
//inline void  operator delete  (void*, void*) throw() { }
//inline void  operator delete[](void*, void*) throw() { }
void * operator new(size_t size)
{
//    std::cout << "New operator overloading " << std::endl;
    void * p = (void *)0;//malloc(size);
    return p;
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
//void operator delete[](void *p) noexcept
//{
//    operator delete(p); // Same as regular delete
//}

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
    A(int _a=5, int _b=6) : a{_a}, b{_b} {

    }
};

int main() {
//    std::cout <<std::to_string(_LIBCPP_STD_VER) << std::endl;
    auto * mem = new uint8_t [sizeof (A)];
    A * p = new(mem) A();
    auto * mem2 = new A;
//    auto * mem3 = ((void *)mem2) A(7,8);

//    std::cout << mem2->a;
//    std::cout << p->a;
}

