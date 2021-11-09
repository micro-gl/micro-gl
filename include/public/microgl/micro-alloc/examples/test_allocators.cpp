#define DEBUG_ALLOCATOR

#include <micro-alloc/dynamic_memory.h>
#include <micro-alloc/pool_memory.h>
#include <micro-alloc/linear_memory.h>
#include <micro-alloc/stack_memory.h>
#include <micro-alloc/std_memory.h>

void test_stack_allocator() {
    using byte= unsigned char;
    const int size = 5000;
    byte memory[size];

    stack_memory<> alloc{memory, size};

    void * a1 = alloc.malloc(5000);
    void * a2 = alloc.malloc(512);
    void * a3 = alloc.malloc(256);
    void * a4 = alloc.malloc(128);
    void * a5 = alloc.malloc(3);
    alloc.free(a5);
    alloc.free(a4);
    alloc.free(a3);
    alloc.free(a2);
    alloc.free(a2);
    alloc.free(a1);
    alloc.free(a1);
        void * a41 = alloc.malloc(200);
        void * a51 = alloc.malloc(200);
        void * a61 = alloc.malloc(200);
        alloc.free(a41);
        alloc.free(a51);
        alloc.free(a61);
    //    alloc.free(a3);
    //    alloc.free(a3);
    //    alloc.free(a3);

}

void test_dynamic_allocator() {
    using byte= unsigned char;
    const int size = 5000;
    byte memory[size];

    dynamic_memory<> alloc{memory, size};

    void * a1 = alloc.malloc(200);
    void * a2 = alloc.malloc(200);
    void * a3 = alloc.malloc(200);
    alloc.free(a3);
    alloc.free(a1);
    alloc.free(a2);
//    alloc.free(a2);
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

    pool_memory<> alloc{memory, size, 256, 8, true};

    void  * p1 = alloc.malloc();
    void  * p2 = alloc.malloc();
    void  * p3 = alloc.malloc();
    void  * p4 = alloc.malloc();
    void  * p5 = alloc.malloc();

    alloc.free(p1);
    alloc.free(p2);
    alloc.free(p4);
    alloc.free(p3);
    alloc.free(p3);
    //    alloc.free((void *)(memory+256));

    //    alloc.free(p1);
    //    alloc.free(p1);
}

void test_linear_allocator() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    linear_memory<> alloc{memory, size};

    void  * p1 = alloc.malloc(512);
    void  * p2 = alloc.malloc(512);
    void  * p3 = alloc.malloc(512);

    alloc.free(p1);
    alloc.free(p2);
    alloc.free(p3);
    alloc.free(p3);
    alloc.free(p3);
//    alloc.reset();

    void  * p4 = alloc.malloc(512);
    void  * p5 = alloc.malloc(512);

    //    alloc.free((void *)(memory+256));

    //    alloc.free(p1);
    //    alloc.free(p1);
    linear_memory<> alloc2{memory, size};

    bool test_equal = alloc.is_equal(alloc2);

}

void test_std_allocator() {

    std_memory alloc;

    void  * p1 = alloc.malloc(512);
    void  * p2 = alloc.malloc(512);
    void  * p3 = alloc.malloc(512);

    alloc.free(p1);
    alloc.free(p2);
    alloc.free(p3);
//    alloc.reset();

    void  * p4 = alloc.malloc(512);
    void  * p5 = alloc.malloc(512);

    //    alloc.free((void *)(memory+256));

    //    alloc.free(p1);
    //    alloc.free(p1);
}


int main() {
    test_std_allocator();
    test_stack_allocator();
    test_dynamic_allocator();
    test_pool_allocator();
    test_linear_allocator();
}
