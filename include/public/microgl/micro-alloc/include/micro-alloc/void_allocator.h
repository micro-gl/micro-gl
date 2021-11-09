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
