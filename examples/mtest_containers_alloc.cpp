#define DEBUG_ALLOCATOR

#include <vector>
#include <microgl/allocators/dynamic_memory.h>
#include <microgl/allocators/pool_memory.h>
#include <microgl/allocators/linear_memory.h>
#include <microgl/allocators/stack_memory.h>
#include <microgl/allocators/std_memory.h>
#include <microgl/allocators/polymorphic_allocator.h>
#include <microgl/dynamic_array.h>
#include <memory>

struct test_t {
    int a;
    char b;
    explicit test_t(int $a=5, char $b=2) : a{$a}, b{$b} {
    }
};

template<typename Type, class Allocator, template<class T, class A> class Container>
void test_container(Allocator & allocator) {
    using container_t = Container<Type, Allocator>;
    container_t container{allocator};

    container.push_back(test_t{1,1});
    container.push_back(test_t{2,2});
    container.push_back(test_t{3,3});
    container.push_back(test_t{4,4});
    container.pop_back();
    container.push_back(test_t{5,5});
    container.pop_back();
    container.push_back(test_t{6,6});
    container.pop_back();
    container.push_back(test_t{7,7});
    container.pop_back();
    container.push_back(test_t{8,8});

    container_t container2{container.get_allocator()};
//    container2 = std::move(container);
//    container_t container3 = std::move(container2);
}

template <template<class T, class A> class Container>
void test_linear_allocator1() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    linear_memory<> resource{memory, size, alignof(size_t)};
    polymorphic_allocator<test_t> allocator{&resource};

    test_container<test_t, polymorphic_allocator<test_t>, Container>(allocator);
}

template <template<class T, class A> class Container>
void test_dynamic_allocator1() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    dynamic_memory<> resource{memory, size};
    polymorphic_allocator<test_t> allocator{&resource};
    test_container<test_t, polymorphic_allocator<test_t>, Container>(allocator);
}

template <template<class T, class A> class Container>
void test_stack_allocator1() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    stack_memory<> resource{memory, size};
    polymorphic_allocator<test_t> allocator{&resource};
    test_container<test_t, polymorphic_allocator<test_t>, Container>(allocator);
}

template <template<class T, class A> class Container>
void test_pool_allocator1() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    pool_memory<> resource{memory, size, sizeof (test_t), true};
    polymorphic_allocator<test_t> allocator{&resource};
    test_container<test_t, polymorphic_allocator<test_t>, Container>(allocator);
}

template <template<class T, class A> class Container>
void test_std_allocator1() {
    using byte= unsigned char;
    const int size = 1024;
    byte memory[size];

    std_memory resource{sizeof (test_t)};
    polymorphic_allocator<test_t> allocator{&resource};
    test_container<test_t, polymorphic_allocator<test_t>, Container>(allocator);
}

int main() {
    test_linear_allocator1<dynamic_array>();
//    test_dynamic_allocator1<dynamic_array>();
//    test_stack_allocator1<dynamic_array>();
//    test_pool_allocator1<dynamic_array>();
//    test_std_allocator1<dynamic_array>();
//
}
