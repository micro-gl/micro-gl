/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "memory_resource.h"

namespace poly_alloc_traits {
    template<class T> struct remove_reference      {typedef T type;};
    template<class T> struct remove_reference<T&>  {typedef T type;};
    template<class T> struct remove_reference<T&&> {typedef T type;};

    template <class _Tp> inline typename remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept
    {
        typedef typename remove_reference<_Tp>::type _Up;
        return static_cast<_Up&&>(__t);
    }

    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type& __t) noexcept
    { return static_cast<_Tp&&>(__t); }

    template <class _Tp> inline _Tp&&
    forward(typename remove_reference<_Tp>::type&& __t) noexcept
    { return static_cast<_Tp&&>(__t); }

    template <class Tp, Tp _v>
    struct integral_constant
    {
        static constexpr const Tp      value = _v;
        typedef Tp               value_type;
        typedef integral_constant type;
        constexpr operator value_type() const noexcept {return value;}
    };

    typedef integral_constant<bool, true> true_type;
    typedef integral_constant<bool, false> false_type;
}

/**
 * polymorphic allocator, that uses exchangeable memory resources
 * @tparam T the allocated object type
 * @tparam uintptr_type per system unsigned integral type that is the size of a pointer
 */
template<typename T=char, typename uintptr_type=unsigned long>
class polymorphic_allocator {
public:
    using value_type = T;
    using memory = memory_resource<uintptr_type>;
    using size_t = uintptr_type;
    static const uintptr_type default_align = sizeof(uintptr_type);

private:
    memory * _mem;

public:
    polymorphic_allocator()=delete;

    template<class U>
    explicit polymorphic_allocator(const polymorphic_allocator<U, uintptr_type> & other) noexcept
                    : polymorphic_allocator{other.resource()} {}
    explicit polymorphic_allocator(memory_resource<uintptr_type> * r) : _mem{r} {}

    memory * resource() const { return _mem; }

    template <class U, class... Args> void construct(U* p, Args&&... args) {
        new(p) U(poly_alloc_traits::forward<Args>(args)...);
    }

    T * allocate(size_t n) { return (T *)_mem->malloc(n * sizeof(T)); }
    void deallocate(T * p, size_t n=0) { _mem->free(p); }

    void* allocate_bytes(size_t nbytes, size_t alignment = default_align) {
        return _mem->malloc(nbytes);
    }

    void deallocate_bytes(void* p, size_t nbytes, size_t alignment = default_align) {
        _mem->free(p);
    }

    template<class U>
    U* allocate_object(size_t n = 1) {
        return allocate_bytes(n * sizeof(U), alignof(U));
    }

    template <class U> void deallocate_object(U * p, size_t n = 1) {
        deallocate_bytes(p, sizeof(U), alignof(U));
    }

    template<class U, class... CtorArgs>
    U* new_object(CtorArgs&&... ctor_args) {
        U * p = allocate_object<U>();
        construct(p, poly_alloc_traits::forward<CtorArgs>(ctor_args)...);
        return p;
    }

    template <class U>
    void delete_object(U* p) {
        p->~U();
        deallocate_object(p);
    }

    polymorphic_allocator & operator=(polymorphic_allocator) = delete;

    polymorphic_allocator select_on_container_copy_construction(const polymorphic_allocator & dummy) const {
        return polymorphic_allocator<T, uintptr_type>(*this);
    }

    template<class U> struct rebind {
        typedef polymorphic_allocator<U, uintptr_type> other;
    };
};

template<class T1, class T2, typename uintptr_type>
bool operator==( const polymorphic_allocator<T1, uintptr_type>& lhs,
                 const polymorphic_allocator<T2, uintptr_type>& rhs ) noexcept {
    return *(lhs.resource()) == *(rhs.resource());
}