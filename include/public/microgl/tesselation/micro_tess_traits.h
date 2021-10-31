#pragma once

namespace microtess {
    namespace traits {

        template< class T > struct remove_reference      {typedef T type;};
        template< class T > struct remove_reference<T&>  {typedef T type;};
        template< class T > struct remove_reference<T&&> {typedef T type;};

        template <class _Tp> inline
        typename remove_reference<_Tp>::type&&
        move(_Tp&& __t) noexcept
        {
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

        // is same
        template<class T1, class T2>
        struct is_same {
            const static bool value = false;
        };
        template<class T>
        struct is_same<T, T> {
            const static bool value = true;
        };

        // is float point
        template<class number>
        constexpr bool is_float_point() {
            return is_same<float, number>::value ||
                   is_same<double, number>::value ||
                   is_same<long double, number>::value;
        }

        // conditional
        template<bool B, class T, class F>
        struct conditional {
            typedef T type;
        };
        template<class T, class F>
        struct conditional<false, T, F> {
            typedef F type;
        };

        template<bool, class _Tp = void> struct enable_if {};
        template<class _Tp> struct enable_if<true, _Tp> { typedef _Tp type; };

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

        // allocator aware traits with SFINAE
        template <typename T, typename = int>
        struct is_allocator_aware : microtess::traits::false_type { };

        template <typename T>
        struct is_allocator_aware <T, decltype((void) T().get_allocator(), 0)> : microtess::traits::true_type { };

    }
}