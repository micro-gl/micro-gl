#pragma once

/**
 * void allocator, does not allocate anything
 * @tparam T the allocated object type
 */
template<typename T=unsigned char>
class void_allocator {
public:
    using value_type = T;
    using size_t = unsigned long;

    template<class U>
    explicit void_allocator(const void_allocator<U> & other) noexcept { };
    explicit void_allocator()=default;
    template <class U, class... Args> void construct(U* p, Args&&... args) {}
    T * allocate(size_t n) { return nullptr; }
    void deallocate(T * p, size_t n=0) {}
    template<class U> struct rebind { typedef void_allocator<U> other; };
};

template<class T1, class T2>
bool operator==(const void_allocator<T1>& lhs, const void_allocator<T2>& rhs ) noexcept {
    return true;
}
