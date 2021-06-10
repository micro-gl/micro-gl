#include <microgl/allocators/dynamic_allocator.h>
#include <microgl/allocators/pool_allocator.h>
#include <iostream>
#include <new>

int test_dynamic_allocator() {
    using byte= unsigned char;
    const int size = 5000;
    byte memory[size];

    dynamic_allocator<> alloc{memory, size};

    void * a1 = alloc.allocate(200);
    void * a2 = alloc.allocate(200);
    void * a3 = alloc.allocate(200);
    void * a4 = alloc.allocate(200);
    void * a5 = alloc.allocate(200);
    void * a6 = alloc.allocate(200);
    alloc.free(a2);
    alloc.free(a4);
    alloc.free(a6);
    alloc.free(a3);
}



int main() {
    test_dynamic_allocator();
}
