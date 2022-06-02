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

namespace microtess {

    namespace traits {
        // integral constant
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

        template <class _Tp> struct is_lvalue_reference       : public false_type {};
        template <class _Tp> struct is_lvalue_reference<_Tp&> : public true_type {};

        template <class _Tp> struct is_rvalue_reference        : public false_type {};
        template <class _Tp> struct is_rvalue_reference<_Tp&&> : public true_type {};

        template <class _Tp> struct is_reference        : public false_type {};
        template <class _Tp> struct is_reference<_Tp&>  : public true_type {};
        template <class _Tp> struct is_reference<_Tp&&> : public true_type {};

        template< class T > struct remove_reference      {typedef T type;};
        template< class T > struct remove_reference<T&>  {typedef T type;};
        template< class T > struct remove_reference<T&&> {typedef T type;};
        template <class T>
        using remove_reference_t = typename remove_reference<T>::type;

        template< class T > struct remove_const_and_reference      {typedef T type;};
        template< class T > struct remove_const_and_reference<const T>  {typedef T type;};
        template< class T > struct remove_const_and_reference<const T&>  {typedef T type;};
        template< class T > struct remove_const_and_reference<T&&> {typedef T type;};
        template <class T>
        using remove_const_and_reference_t = typename remove_reference<T>::type;

        template< class T > struct remove_const      {typedef T type;};
        template< class T > struct remove_const<const T>  {typedef T type;};
        template< class T > struct remove_const<const T&>  {typedef T & type;};
        template< class T > struct remove_const<T&&> {typedef T&& type;};
        template <class T>
        using remove_const_t = typename remove_reference<T>::type;


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
            static_assert(!is_lvalue_reference<_Tp>::value,
                          "can not forward an rvalue as an lvalue");
            return static_cast<_Tp&&>(__t);
        }

        // is same
        template<class T1, class T2> struct is_same { constexpr static bool value = false; };
        template<class T> struct is_same<T, T> { constexpr static bool value = true; };

        // is float point
        template<class number>
        constexpr bool is_float_point() {
            return is_same<float, number>::value ||
                   is_same<double, number>::value ||
                   is_same<long double, number>::value;
        }

        // conditional
        template<bool B, class T, class F>
        struct conditional { typedef T type; };
        template<class T, class F>
        struct conditional<false, T, F> { typedef F type; };

        template<bool, class _Tp = void> struct enable_if {};
        template<class _Tp> struct enable_if<true, _Tp> { typedef _Tp type; };
        template< bool B, class T = void >
        using enable_if_t = typename enable_if<B,T>::type;

        // allocator aware traits with SFINAE
        template <typename T, typename = int>
        struct is_allocator_aware : microtess::traits::false_type { };

        template <typename T>
        struct is_allocator_aware <T, typename conditional<false, typename T::allocator_type, int>::type> :
                microtess::traits::true_type { };

        // another version with decltype
//        template <typename T>
//        struct is_allocator_aware <T, decltype((void) T().get_allocator(), 0)> : microtess::traits::true_type { };

        template<class T> struct is_integral { constexpr static bool value = false; };
        template<> struct is_integral<unsigned> { constexpr static bool value = true; };
        template<> struct is_integral<signed> { constexpr static bool value = true; };
        template<> struct is_integral<char> { constexpr static bool value = true; };

        template<typename _Tp, typename _Up = _Tp&&>
        _Up __declval(int);  // (1)

        template<typename _Tp>
        _Tp __declval(long); // (2)

        template<typename _Tp>
        auto declval() noexcept -> decltype(__declval<_Tp>(0));

    }
}

namespace microtess {

    using uintptr_type = typename traits::conditional<
            sizeof(void *) == sizeof(unsigned short), unsigned short,
            typename traits::conditional<
                    sizeof(void *) == sizeof(unsigned int), unsigned int,
                    typename traits::conditional<
                            sizeof(void *) ==
                            sizeof(unsigned long), unsigned long, unsigned long long>::type>::type>::type;

    using ptrdiff_type = typename traits::conditional<
            sizeof(void *) == sizeof(unsigned short), short,
            typename traits::conditional<
                    sizeof(void *) == sizeof(unsigned int), int,
                    typename traits::conditional<
                            sizeof(void *) ==
                            sizeof(unsigned long), long, long long>::type>::type>::type;

    using size_t = uintptr_type;
    using ptrdiff_t = ptrdiff_type;
}

enum class microtess_new { blah };
// This is a placement new override
inline void* operator new (microtess::size_t n, void* ptr, enum microtess_new) noexcept {
    return ptr;
}

namespace microtess {
    template<typename T>
    class std_allocator {
    public:
        using value_type = T;
        using size_t = microtess::size_t;
    public:
        template<class U>
        explicit std_allocator(const std_allocator<U> & other) noexcept { };
        explicit std_allocator()=default;
        template <class U, class... Args> void construct(U* p, Args&&... args)
                { ::new(p, microtess_new::blah) U(traits::forward<Args>(args)...); }
        T * allocate(size_t n) { return (T *)operator new(n * sizeof(T)); }
        void deallocate(T * p, size_t n=0) { operator delete (p); }
        template<class U> struct rebind { typedef std_allocator<U> other; };
    };

    template<class T1, class T2>
    bool operator==(const std_allocator<T1>& lhs, const std_allocator<T2>& rhs ) noexcept {
        return true;
    }

}

//namespace microtess {
//    namespace traits {
//
//        template< class T > struct remove_reference      {typedef T type;};
//        template< class T > struct remove_reference<T&>  {typedef T type;};
//        template< class T > struct remove_reference<T&&> {typedef T type;};
//
//        template <class _Tp> inline typename remove_reference<_Tp>::type&&
//        move(_Tp&& __t) noexcept {
//            typedef typename remove_reference<_Tp>::type _Up;
//            return static_cast<_Up&&>(__t);
//        }
//        template <class _Tp> inline _Tp&&
//        forward(typename remove_reference<_Tp>::type& __t) noexcept {
//            return static_cast<_Tp&&>(__t);
//        }
//        template <class _Tp> inline _Tp&&
//        forward(typename remove_reference<_Tp>::type&& __t) noexcept {
//            return static_cast<_Tp&&>(__t);
//        }
//
//        // is same
//        template<class T1, class T2> struct is_same { const static bool value = false; };
//        template<class T> struct is_same<T, T> { const static bool value = true; };
//
//        // is float point
//        template<class number>
//        constexpr bool is_float_point() {
//            return is_same<float, number>::value ||
//                   is_same<double, number>::value ||
//                   is_same<long double, number>::value;
//        }
//
//        // conditional
//        template<bool B, class T, class F>
//        struct conditional { typedef T type; };
//        template<class T, class F>
//        struct conditional<false, T, F> { typedef F type; };
//
//        template<bool, class _Tp = void> struct enable_if {};
//        template<class _Tp> struct enable_if<true, _Tp> { typedef _Tp type; };
//
//        // integral constant
//        template <class Tp, Tp _v>
//        struct integral_constant
//        {
//            static constexpr const Tp      value = _v;
//            typedef Tp               value_type;
//            typedef integral_constant type;
//            constexpr operator value_type() const noexcept {return value;}
//        };
//
//        typedef integral_constant<bool, true> true_type;
//        typedef integral_constant<bool, false> false_type;
//
//        // allocator aware traits with SFINAE
//        template <typename T, typename = int>
//        struct is_allocator_aware : microtess::traits::false_type { };
//
//        template <typename T>
//        struct is_allocator_aware <T, decltype((void) T().get_allocator(), 0)> : microtess::traits::true_type { };
//    }
//}