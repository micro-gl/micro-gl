#pragma once

namespace microtess {

    namespace std_rebind_allocator_traits {
        template< class T > struct remove_reference      {typedef T type;};
        template< class T > struct remove_reference<T&>  {typedef T type;};
        template< class T > struct remove_reference<T&&> {typedef T type;};

        template <class _Tp> inline typename remove_reference<_Tp>::type&&
        move(_Tp&& __t) noexcept {
            typedef typename remove_reference<_Tp>::type _Up;
            return static_cast<_Up&&>(__t);
        }
        template <class _Tp> inline _Tp&&
        forward(typename remove_reference<_Tp>::type& __t) noexcept {
            return static_cast<_Tp&&>(__t);
        }
        template <class _Tp> inline _Tp&&
        forward(typename remove_reference<_Tp>::type&& __t) noexcept {
            return static_cast<_Tp&&>(__t);
        }
    }

    /**
     * standard allocator
     * @tparam T the allocated object type
     */
    template<typename T=unsigned char>
    class std_rebind_allocator {
    public:
        using value_type = T;
        using size_t = unsigned long;

        template<class U>
        explicit std_rebind_allocator(const std_rebind_allocator<U> & other) noexcept { };
        explicit std_rebind_allocator()=default;

        template <class U, class... Args>
        void construct(U* p, Args&&... args) {
            new(p) U(std_rebind_allocator_traits::forward<Args>(args)...);
        }

        T * allocate(size_t n) { return (T *)operator new(n * sizeof(T)); }
        void deallocate(T * p, size_t n=0) { operator delete (p); }

        template<class U> struct rebind {
            typedef std_rebind_allocator<U> other;
        };
    };

    template<class T1, class T2>
    bool operator==(const std_rebind_allocator<T1>& lhs, const std_rebind_allocator<T2>& rhs ) noexcept {
        return true;
    }
}
