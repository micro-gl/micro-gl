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
 * memory pool block allocator
 * free is:
 * - O(1) when {guard_against_double_free==false} (in constructor)
 * - O(free-list-size) when {guard_against_double_free==true} (in constructor)
 *
 * allocations are O(1)
 *
 * minimal block size is 4 bytes for 32 bit pointer types and 8 bytes for 64 bits pointers.
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
class pool_memory : public memory_resource<uintptr_type> {
private:
    using base = memory_resource<uintptr_type>;
    using typename base::uptr;
    using typename base::uint;
    using base::align_up;
    using base::align_down;
    using base::is_aligned;
    using base::ptr_to_int;
    using base::int_to_ptr;

    template<typename T>
    static T int_to(uptr integer) { return reinterpret_cast<T>(integer); }

    struct header_t {
        header_t * next=nullptr;
    };

    void * _ptr=nullptr;
    uint _size=0;
    uint _block_size=0;
    uint _blocks_count=0;
    uint _free_blocks_count=0;
    header_t * _free_list_root = nullptr;
    bool _guard_against_double_free=false;

    uint minimal_size_of_any_block() const {
        return align_up(sizeof (header_t));
    }

    uint correct_block_size(uint block_size) const {
        block_size = align_up(block_size);
        if(block_size < minimal_size_of_any_block())
            block_size = minimal_size_of_any_block();
        return block_size;
    }

    uint compute_blocks_count() {
        uptr a = align_up(ptr_to_int(_ptr));
        uptr b = align_down(ptr_to_int(_ptr)+_size);
        uptr diff = b-a;
        return diff/_block_size;
    }

public:
    uint blocks_count() {
        return _blocks_count;
    }

    uint free_blocks_count() const {
        return _free_blocks_count;
    }

    uptr start_aligned_address() const {
        return align_up(ptr_to_int(_ptr));
    }

    uptr end_aligned_address() const {
        return align_down(ptr_to_int(_ptr) + _size);
    }

    uptr available_size() const override {
        return free_blocks_count()*_block_size;
    }

    pool_memory()=delete;

    /**
     *
     * @param ptr start of memory
     * @param size_bytes the memory size in bytes
     * @param block_size the block size
     * @param guard_against_double_free if {True}, user will not be able to accidentally
     *          free an already free block at the cost of having free operation at O(free-list-size).
     *          If {False}, free will take O(1) operations like allocations.
     */
    pool_memory(void * ptr, uint size_bytes, uint block_size,
                uptr alignment=sizeof (uintptr_type),
                bool guard_against_double_free=false) :
            base{3, alignment}, _ptr(ptr), _size(size_bytes), _block_size(0),
            _guard_against_double_free(guard_against_double_free) {
        const bool is_memory_valid_1 = correct_block_size(block_size) <= size_bytes;
        const bool is_memory_valid_2 = sizeof(void *)==sizeof(uintptr_type);
        const bool is_memory_valid_3 = alignment % sizeof(uintptr_type)==0;
        const bool is_memory_valid = is_memory_valid_1 and is_memory_valid_2 and is_memory_valid_3;
        if(is_memory_valid) reset(block_size);
        this->_is_valid = is_memory_valid;

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "HELLO:: pool memory resource"<< std::endl;
        std::cout << "* correct block size due to headers and alignment is "
                  << correct_block_size(block_size) << " bytes" <<std::endl;
        std::cout << "* requested alignment is " << alignment << " bytes" << std::endl;
        if(is_memory_valid)
            std::cout << "* first block @ " << ptr_to_int(_free_list_root) << std::endl;
        if(!is_memory_valid_1)
            std::cout << "* memory does not satisfy minimal size requirements !!!"
                      << std::endl;
        if(!is_memory_valid_2)
            std::cout << "* error:: a pointer is not expressible as uintptr_type !!!"
                      << std::endl;
        if(!is_memory_valid_3)
            std::cout << "* error:: alignment should be a power of 2 divisible by sizeof(uintptr_type)="
                      << sizeof(uintptr_type) << " !!!" << std::endl;
        print(false);
#endif
    }

    ~pool_memory() override {
        _free_list_root=nullptr;
        _ptr=nullptr;
        _blocks_count=_block_size=_size=0;
    }

    void reset(const uint block_size) {
        _block_size = correct_block_size(block_size);
        const uint blocks = _free_blocks_count = _blocks_count
                = compute_blocks_count();
        void * ptr = _ptr;
        uptr current = align_up(ptr_to_int(ptr));
        uptr next = current + block_size;
        _free_list_root = int_to<header_t *>(current);
        for (int ix = 0; ix < blocks-1; ++ix) {
            auto * header_current = int_to<header_t *>(current);
            auto * header_next = int_to<header_t *>(next);
            header_current->next = header_next;
            current += _block_size;
            next += _block_size;
        }
        int_to<header_t *>(current)->next = nullptr;
    }

    void * malloc(uptr size_bytes_dont_matter=0) override {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "MALLOC:: pool memory resource"
                  << std::endl;
#endif

        if(_free_list_root==nullptr) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- no free blocks are available" << std::endl;
#endif
            return nullptr;
        }
        auto * current_node = _free_list_root;
        _free_list_root = _free_list_root->next;
        _free_blocks_count-=1;

#ifdef DEBUG_ALLOCATOR
        std::cout << "- handed a free block @"
                  << ptr_to_int(current_node) << std::endl;
        std::cout << "- free blocks in pool [" << _free_blocks_count << "/"
        << _blocks_count << "]" << std::endl;
#endif

        return current_node;
    }

    bool free(void * pointer) override {
        auto address = ptr_to_int(pointer);
        const uptr min_range = align_up(ptr_to_int(_ptr));
        const uptr max_range = align_down(ptr_to_int(_ptr) + _size);
        const bool is_in_range = address >= min_range && address < max_range;

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "FREE:: pool allocator " << std::endl
                  << "- free a block address @ " << address << std::endl;
#endif
        if(!is_in_range) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error: address is not in range [" << min_range
                      << " -- " << max_range << std::endl;
#endif
            return false;
        }

        bool is_address_block_aligned = (address-min_range)%_block_size==0;
        if(!is_address_block_aligned) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error: address is not aligned to " << _block_size
                      << " bytes block sizes" << std::endl;
#endif
            return false;
        }

        if(_guard_against_double_free) {
            bool is_freeing_an_already_free_block = false;
            auto * current = _free_list_root;
            while (current) {
                if(ptr_to_int(current)==address) {
                    is_freeing_an_already_free_block=true;
                    break;
                }
                current=current->next;
            }
            if(is_freeing_an_already_free_block) {
#ifdef DEBUG_ALLOCATOR
                std::cout << "- error: tried to free an already Free block" << std::endl;
#endif
                return false;
            }
        }

        auto * block = int_to<header_t *>(address);
        block->next = _free_list_root;
        _free_list_root = block;
        _free_blocks_count+=1;

#ifdef DEBUG_ALLOCATOR
        std::cout << "- free blocks in pool [" << _free_blocks_count << "/"
                  << _blocks_count << "]" << std::endl;
#endif

        return true;
    }

    void print(bool dummy) const override {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "PRINT:: pool allocator " << std::endl;
        std::cout << "- free list is [" << _free_blocks_count << "/" << _blocks_count << "]" << std::endl;
        std::cout << std::endl;
#endif
    }

    bool is_equal(const memory_resource<> &other) const noexcept override {
        bool equals = this->type_id() == other.type_id();
        if(!equals) return false;
        const auto * casted_other = reinterpret_cast<const pool_memory<> *>(&other);
        equals = this->_ptr==casted_other->_ptr;
        return equals;
    }

};
