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

namespace microgl {
    namespace traits {

        template<class T1, class T2> struct is_same { const static bool value = false; };
        template<class T> struct is_same<T, T> { const static bool value = true; };

        template<class number>
        constexpr bool is_float_point() {
            return is_same<float, number>::value ||
                   is_same<double, number>::value ||
                   is_same<long double , number>::value;
        }

        template<bool B, class T, class F> struct conditional { typedef T type; };
        template<class T, class F> struct conditional<false, T, F> { typedef F type; };

        template <bool, class _Tp = void> struct enable_if {};
        template <class _Tp> struct enable_if<true, _Tp> {typedef _Tp type;};

        template< class T > struct remove_reference      {typedef T type;};
        template< class T > struct remove_reference<T&>  {typedef T type;};
        template< class T > struct remove_reference<T&&> {typedef T type;};

        template <class _Tp> inline typename remove_reference<_Tp>::type&&
        move(_Tp&& __t) noexcept {
            typedef typename remove_reference<_Tp>::type _Up;
            return static_cast<_Up&&>(__t);
        }

        template <class _Tp> inline _Tp&&
        forward(typename remove_reference<_Tp>::type& __t) noexcept
        { return static_cast<_Tp&&>(__t); }

        template <class _Tp> inline _Tp&&
        forward(typename remove_reference<_Tp>::type&& __t) noexcept
        { return static_cast<_Tp&&>(__t); }

        template <typename T> class crpt {
        protected:
            T& derived() { return static_cast<T&>(*this); }
            T const& derived() const { return static_cast<T const&>(*this); }
        };

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
                new(p) U(forward<Args>(args)...);
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
}