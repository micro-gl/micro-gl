#define DEBUG_ALLOCATOR

#include <microgl/allocators/dynamic_allocator.h>
#include <microgl/allocators/pool_allocator.h>
#include <microgl/allocators/linear_allocator.h>
#include <microgl/allocators/stack_allocator.h>
#include <iostream>
#include <new>

void test_stack_allocator() {
    using byte= unsigned char;
    const int size = 5000;
    byte memory[size];

    stack_allocator<> alloc{memory, size};

    void * a1 = alloc.allocate(5000);
    void * a2 = alloc.allocate(512);
    void * a3 = alloc.allocate(256);
    void * a4 = alloc.allocate(128);
    void * a5 = alloc.allocate(3);
    alloc.free(a5);
    alloc.free(a4);
    alloc.free(a3);
    alloc.free(a2);
    alloc.free(a2);
    alloc.free(a1);
    alloc.free(a1);
    //    void * a4 = alloc.allocate(200);
    //    void * a5 = alloc.allocate(200);
    //    void * a6 = alloc.allocate(200);
    //    alloc.free(a2);
    //    alloc.free(a4);
    //    alloc.free(a6);
    //    alloc.free(a3);
    //    alloc.free(a3);
    //    alloc.free(a3);

}

void test_dynamic_allocator() {
    using byte= unsigned char;
    const int size = 5000;
    byte memory[size];

    dynamic_allocator<> alloc{memory, size};

    void * a1 = alloc.allocate(200);
    void * a2 = alloc.allocate(200);
    void * a3 = alloc.allocate(200);
    alloc.free(a3);
    alloc.free(a3);
//    void * a4 = alloc.allocate(200);
//    void * a5 = alloc.allocate(200);
//    void * a6 = alloc.allocate(200);
//    alloc.free(a2);
//    alloc.free(a4);
//    alloc.free(a6);
//    alloc.free(a3);
//    alloc.free(a3);
//    alloc.free(a3);

}

void test_pool_allocator() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    pool_allocator<> alloc{memory, size, 256, true};

    void  * p1 = alloc.allocate();
    void  * p2 = alloc.allocate();
    void  * p3 = alloc.allocate();
    void  * p4 = alloc.allocate();
    void  * p5 = alloc.allocate();

    alloc.free(p1);
    alloc.free(p1);
    //    alloc.free((void *)(memory+256));

    //    alloc.free(p1);
    //    alloc.free(p1);
}

void test_linear_allocator() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    linear_allocator<> alloc{memory, size};

    void  * p1 = alloc.allocate(512);
    void  * p2 = alloc.allocate(512);
    void  * p3 = alloc.allocate(512);

    alloc.free(p1);
    alloc.free(p1);
    alloc.reset();

    void  * p4 = alloc.allocate(512);
    void  * p5 = alloc.allocate(512);

    //    alloc.free((void *)(memory+256));

    //    alloc.free(p1);
    //    alloc.free(p1);
}



int main() {
    test_stack_allocator();
//    test_dynamic_allocator();
//    test_pool_allocator();
//    test_linear_allocator();
}
