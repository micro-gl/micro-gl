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

//#define DEBUG_ALLOCATOR

#ifdef DEBUG_ALLOCATOR
#include <iostream>
#endif

/**
 * linear memory allocator:
 *
 * memory is given progressively without freeing. user can only reset the pointer to the
 * beginning. this memory is not shrinking.
 *
 * - allocations are O(1)
 * - free does not do anything
 *
 * @tparam uintptr_type unsigned integer type that can hold a pointer
 * @tparam alignment alignment requirement, must be valid power of 2, that can satisfy
 *         the highest alignment requirement that you wish to store in the memory dynamic_memory.
 *         alignment of atomic types usually equals their size.
 *         alignment of struct types equals the maximal alignment among it's member types.
 *         if you have std lib, you can infer these, otherwise, just plug them if you know
 *
 * @author Tomer Riko Shalev
 */
template<typename uintptr_type=unsigned long>
class linear_memory : public memory_resource<uintptr_type> {
private:
    using base = memory_resource<uintptr_type>;
    using typename base::uptr;
    using typename base::uint;
    using base::align_up;
    using base::align_down;
    using base::is_aligned;
    using base::ptr_to_int;
    using base::int_to_ptr;
    using base::int_to;

    void * _ptr;
    void * _current_ptr=nullptr;
    uint _size=0;

public:

    linear_memory()=delete;
    /**
     *
     * @param ptr start of memory
     * @param size_bytes the memory size in bytes
     * @param block_size the block size
     * @param guard_against_double_free if {True}, user will not be able to accidentally
     *          free an already free block at the cost of having free operation at O(free-list-size).
     *          If {False}, free will take O(1) operations like allocations.
     */
    linear_memory(void * ptr, uint size_bytes, uptr alignment=sizeof (uintptr_type)) :
            base{1, alignment}, _ptr(ptr), _size(size_bytes) {
        const bool is_memory_valid_1 = sizeof(void *)==sizeof(uintptr_type);
        const bool is_memory_valid_2 = alignment % sizeof(uintptr_type)==0;
        const bool is_memory_valid = is_memory_valid_1 and is_memory_valid_2;
        if(is_memory_valid) reset();
        this->_is_valid = is_memory_valid;

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "HELLO:: linear memory resource"<< std::endl;
        std::cout << "* requested alignment is " << alignment << " bytes" << std::endl;
        std::cout << "* size is " << size_bytes << " bytes" << std::endl;
        if(!is_memory_valid_1)
            std::cout << "* error:: a pointer is not expressible as uintptr_type !!!"
                      << std::endl;
        if(!is_memory_valid_2)
            std::cout << "* error:: alignment should be a power of 2 divisible by sizeof(uintptr_type)="
                      << sizeof(uintptr_type) << " !!!" << std::endl;
#endif
    }

    ~linear_memory() override {
        _current_ptr=_ptr=nullptr;
        _size=0;
    }

    void reset() {
        _current_ptr = base:: template int_to<void *>(align_up(ptr_to_int(_ptr)));

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "RESET:: linear memory" << std::endl
                  << "- reset memory to start @ " << ptr_to_int(_current_ptr)
                  << " (aligned up)" << std::endl;
#endif
    }

    uptr available_size() const override {
        const uptr min = align_up(ptr_to_int(_current_ptr));
        const uptr delta = end_aligned_address() - min;
        return delta;
    }

    uptr start_aligned_address() const {
        return align_up(ptr_to_int(_ptr));
    }

    uptr end_aligned_address() const {
        return align_down(ptr_to_int(_ptr) + _size);
    }

    void * malloc(uptr size_bytes) override {
        size_bytes = align_up(size_bytes);
        const uptr available_space = available_size();
        const bool has_available_size = size_bytes <= available_space;
        const bool has_requested_size_zero = size_bytes==0;

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "MALLOC:: linear allocator" << std::endl
                  << "- request a block of size " << size_bytes << " (aligned up)"
                  << std::endl;
#endif

        if(has_requested_size_zero) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error, cannot fulfill a size 0 bytes block !!"
                      << std::endl;
#endif
            return nullptr;
        }

        if(!has_available_size) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error, could not fulfill this size"
                      << std::endl << "- available size is " << available_space
                      << std::endl;
#endif
            return nullptr;
        }

        auto * pointer = _current_ptr;
        _current_ptr = base:: template int_to<void *>(ptr_to_int(pointer) + size_bytes);

        return pointer;
    }

    bool free(void * pointer) override {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "FREE:: linear allocator "
        << std::endl << "- linear allocator does not free space, use reset() instead "
        << std::endl << "- available size is " << available_size()
        << std::endl;
#endif
        return true;
    }

    void print(bool embed) const override {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "PRINT:: linear allocator "
                  << std::endl << "- available size is " << available_size()
                  << std::endl;
#endif
    }

    bool is_equal(const memory_resource<> &other) const noexcept override {
        bool equals = this->type_id() == other.type_id();
        if(!equals) return false;
        const auto * casted_other = static_cast<const linear_memory *>(&other);
        equals = this->_ptr==casted_other->_ptr;
        return equals;
    }

};
