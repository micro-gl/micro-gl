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

#include "../traits.h"

namespace microgl {
    namespace lut {

        namespace detail {
            template<int... Is> struct seq{};

            template<int N, int... Is>
            struct gen_seq : gen_seq<(N)-(1), (N)-(1), (Is)...>{};

            template<int... Is>
            struct gen_seq<(0), Is...> : seq<(Is)...>{};

            template<>
            struct gen_seq<(0)> : seq<0>{};
        }

        /**
         * dynamic runtime lookup table generation
         * @tparam type the type of element of the LUT
         * @tparam size the size of the table
         * @tparam heap create the lut on stack or heap
         * @tparam mute mute, no data is spent
         */
        template <typename type, unsigned size, bool heap=false, bool mute=false>
        struct dynamic_lut {
        private:
            struct stack_lut {
                type _data[size];
                stack_lut() : _data{0} {}
                const type &operator[](const unsigned index) const {
                    return _data[index];
                }
                type &operator[](const unsigned index) {
                    return _data[index];
                }
            };
            struct heap_lut {
                type *_data;
                heap_lut() : _data{new type[size]} {}
                ~heap_lut() { delete [] _data; }
                type &operator[](const unsigned index) const {
                    return _data[index];
                }
                type &operator[](const unsigned index) {
                    return _data[index];
                }
            };
            struct empty_lut {
                type &operator[](const unsigned index) const {
                    static type dummy{0};
                    return dummy;
                }
                type &operator[](const unsigned index) {
                    static type dummy{0};
                    return dummy;
                }
            };

            typename traits::conditional<heap, heap_lut, typename traits::conditional<!mute,
                        stack_lut, empty_lut>::type>::type lut;
        public:
            explicit dynamic_lut() : lut{} {}

            template <typename function>
            void generate(const function & generator) {
                for (unsigned ix = 0; ix < size; ++ix)
                    lut[ix]=generator(ix);
            }

            type operator[](const unsigned index) const {
                return lut[index];
            }
        };

        /**
         * static compile-time lookup table generation
         * @tparam type the type of element of the LUT
         * @tparam size the size of the table
         * @tparam function the function structure to generate a single element by index
         */
        template <typename type, unsigned size, typename function>
        struct static_lut {
        private:
            struct Table { type data[size]; };

//            template<int... Is> struct seq{};

//            template<int N, int... Is>
//            struct gen_seq : gen_seq<(N)-(1), (N)-(1), (Is)...>{};
//
//            template<int... Is>
//            struct gen_seq<(0), Is...> : seq<(Is)...>{};
//
//            template<>
//            struct gen_seq<(0)> : seq<0>{};

            template<int... Is>
            static constexpr Table generate(detail::seq<Is...>){
                return {{ function::apply(Is)... }};
            }

            constexpr static Table tab=generate(detail::gen_seq<size>{});

        public:
            static type get(const int & n) { return tab.data[n]; }
        };

        // definition
        template <typename type, unsigned size, typename function>
        constexpr typename static_lut<type, size, function>::Table static_lut<type, size, function>::tab;
    }
}
