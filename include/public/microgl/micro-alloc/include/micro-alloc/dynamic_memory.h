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
 * best-fit free list dynamic memory allocator with blocks coalescing.
 * allocation is O(free-list-size)
 * free is:
 * - O(1) when coalescing with neighbor free blocks (because we know exactly where to insert),
 * - O(free-list-size) when we can't coalesce.
 * notes:
 * - free blocks are inserted sorted by their address ascending, this is known to reduce fragmentation
 * - minimal block size 16 bytes for 32 bit pointer types and 32 bytes for 64 bits pointers.
 *
 * Allocated block layout is:
 * [ size|1 | ... payload .... | size|1 ]
 *
 * Free block layout is:
 * [ size|1 | prev | next | ... padding .... | size|1 ]
 *
 * - size includes the whole block size in bytes, it is a power of 2, bigger than 1.
 * - the last bit of size indicates allocation status [1==allocated, 0==free],
 *   last bit is always unused for power of two integers, that are not equal to 1,
 *   and since we have a minimum of 16 bytes block, the last three bits are always unused.
 *
 * Safety:
 * - we assume all free operations are invoked on allocated objects, there are no safe guards
 *   as with my other allocators, as this guard operation might take a lot of time complexity-wise,
 *   so please, pay attention to your de-allocations.
 * - I do perform some easy sanity checks:
 *   - allocated block base header and footer should match, which lowers
 *     the risk of freeing a non-block.
 *   - I test that pointers are aligned to the requested alignment.
 *
 * @tparam uintptr_type unsigned integer type that can hold a pointer
 *
 * @author Tomer Riko Shalev
 */
template<typename uintptr_type=unsigned long>
class dynamic_memory : public memory_resource<uintptr_type> {
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
    uint _size;

    struct base_header_t {
        uptr size_and_status=0;

        uptr size() const {
            return size_and_status & (~(uptr(1)));
        }
        void set_size_and_status(uptr size, bool status) {
            uptr stat = status ? 1 : 0;
            size_and_status = size | stat;
        }
        bool is_allocated() const {
            return size_and_status & 1;
        }
        bool toggle_allocated() {
            return size_and_status ^= (uptr(1));
        }
    };

    using footer_t = base_header_t;

    struct header_t {
        base_header_t base;
        // following fields are for free block
        header_t * prev=nullptr;
        header_t * next=nullptr;
    };

    header_t *_free_list_root = nullptr;

    struct block_t {
        uptr aligned_from=0, aligned_to=0;
        const dynamic_memory<uintptr_type> * allocator;

        header_t * header() const {
            return reinterpret_cast<header_t *>(aligned_from);
        }
        footer_t * footer() const {
            return reinterpret_cast<footer_t *>(aligned_to -
            allocator->align_up(sizeof (footer_t)));
        }
        uptr size() const {
            return header()->base.size();
        }
        void set_size_and_status(uptr size, bool status) const {
            uptr stat = status ? 1 : 0;
            uptr size_and_status = size | stat;
            header()->base.set_size_and_status(size_and_status, status);
            footer()->set_size_and_status(size_and_status, status);
        }
        bool is_allocated() const {
            return header()->base.is_allocated();
        }
        void toggle_allocated() const {
            header()->base.toggle_allocated();
            footer()->toggle_allocated();
        }
        bool sanity_test() const {
            return footer()->size_and_status==header()->base.size_and_status;
        }
    };

    block_t create_free_block(uptr from, uptr to) const {
        block_t result;
        result.aligned_from = align_up(from);
        result.aligned_to = align_down(to);
        result.allocator = this;
        result.set_size_and_status(result.aligned_to-result.aligned_from, false);
        result.header()->prev=nullptr;
        result.header()->next=nullptr;
        return result;
    }

    block_t get_block(uptr from) const {
        block_t result;
        result.aligned_from = align_up(from);
        result.aligned_to = result.aligned_from+result.size();
        result.allocator = this;
        return result;
    }

    static uptr size_of_free_block_header() {
        return sizeof (header_t);
    }

    static uptr size_of_block_base_header() {
        return sizeof (base_header_t);
    }

    static uptr size_of_block_footer() {
        return sizeof (footer_t);
    }

    uint minimal_size_of_any_block() {
        return align_up(size_of_free_block_header()) +
        align_up(size_of_block_footer());
    }

    uint aligned_base_header_and_footer() {
        // minus the next prev pointers
        return (align_up(size_of_block_base_header()) +
        align_up(size_of_block_footer()));
    }

    uint effective_payload_size_of_block(header_t * block) {
        // minus the next prev pointers
        return block->base.size() - aligned_base_header_and_footer();
    }

    uptr compute_required_block_size_by_payload_size(uint payload_size) {
        payload_size = align_up(payload_size);
        auto allocated_block = align_up(size_of_block_base_header()) +
                align_up(size_of_block_footer()) + payload_size;
        auto minimal_free_block_size = minimal_size_of_any_block();
        if(minimal_free_block_size > allocated_block)
            return minimal_free_block_size;
        return allocated_block;
    }

    header_t * split_free_block_to_two_by_payload_size(header_t * block, uint payload_size) {
        payload_size = align_up(payload_size);
        // size of left allocated block
        uptr required_allocated_block_size =
                compute_required_block_size_by_payload_size(payload_size);
        // min acceptable size of right free block
        uptr required_free_block_size = minimal_size_of_any_block() + this->alignment;
        bool has_space = required_allocated_block_size + required_free_block_size
                <= block->base.size();
        if(has_space) {
            // record old data
            auto old_size = block->base.size();
            auto * block_prev = block->prev;
            auto * block_next = block->next;
            // left allocated block
            auto s_b1 = ptr_to_int(block);
            auto e_b1 = ptr_to_int(block)+required_allocated_block_size;
            auto block_1 = create_free_block(s_b1, e_b1);
            // right free block
            auto s_b2 = block_1.aligned_to;
            auto e_b2 = s_b1+old_size;
            auto block_2 = create_free_block(s_b2, e_b2);
            // now re-connect free block to free-list
            block_1.header()->prev = block_prev;
            block_1.header()->next = block_2.header();
            block_2.header()->prev = block_1.header();
            block_2.header()->next = block_next;

#ifdef DEBUG_ALLOCATOR
            std::cout << "- split:: from size [" << old_size << "] bytes (aligned up) "
            << "into two blocks of sizes [" << block_1.size() << ":" << block_2.size()
            << "]" << std::endl;
#endif
            return block_1.header();
        }
        return block;
    }

    uptr _allocations = 0;

public:

    uptr available_size() const override {
        const uptr max = end_aligned_address() - start_aligned_address();
        return max - _allocations;
    }

    uptr start_aligned_address() const {
        return align_up(this->ptr_to_int(_ptr));
    }

    uptr end_aligned_address() const {
        return align_down(this->ptr_to_int(_ptr) + _size);
    }

    dynamic_memory()=delete;

    /**
     * ctor
     *
     * @param ptr pointer of starting pool
     * @param size_bytes amount of bytes
     * @param alignment alignment has to be a power of 2 that is divisible sizeof(uintptr_type)
     */
    dynamic_memory(void * ptr, unsigned int size_bytes, uptr alignment=sizeof (uintptr_type)) :
            base{2, alignment}, _ptr(ptr), _size(size_bytes) {
        const auto block = create_free_block(this->ptr_to_int(ptr), this->ptr_to_int(ptr) + size_bytes);
        const bool is_memory_valid_1 = block.size() >= minimal_size_of_any_block();
        const bool is_memory_valid_2 = sizeof(void *)==sizeof(uintptr_type);
        const bool is_memory_valid_3 = alignment % sizeof(uintptr_type)==0;
        const bool is_memory_valid = is_memory_valid_1 and is_memory_valid_2 and is_memory_valid_3;

        if(is_memory_valid) _free_list_root = block.header();
        this->_is_valid = is_memory_valid;

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "HELLO:: dynamic memory resource"<< std::endl;
        std::cout << "* minimal block size due to headers, footers and alignment is "
                  << minimal_size_of_any_block() << " bytes" <<std::endl;
        std::cout << "* requested alignment is " << this->alignment << " bytes" << std::endl;
        std::cout << "* principal memory block after alignment is " << block.size() << " bytes"
                  << std::endl;

        if(!is_memory_valid_1)
            std::cout << "* error:: memory does not satisfy minimal size requirements !!!"
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

    ~dynamic_memory() override {
        _free_list_root= nullptr;
        _ptr=nullptr;
        _allocations=_size=0;
        this->_is_valid=false;
    }

    void * malloc(uptr size_bytes) override {
        size_bytes = align_up(size_bytes);
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "MALLOC:: dynamic allocator " << std::endl
                  << "- requested block size is " << size_bytes
                  << " bytes (aligned up)" << std::endl;
#endif
        auto * current_node = _free_list_root;
        header_t * best_node= nullptr;
#ifdef DEBUG_ALLOCATOR
        std::cout << "- search :: searching for max(" << size_bytes +
        aligned_base_header_and_footer()
        << ", " << minimal_size_of_any_block() << ") bytes block due to align, "
                                                  "base header and footer"
                                                  << std::endl;
#endif
        while (current_node) {
            bool flag_size_fits = size_bytes <= effective_payload_size_of_block(current_node);
            if(flag_size_fits) {
                bool is_best_fit = best_node == nullptr ||
                        current_node->base.size() < best_node->base.size();
                if(is_best_fit)
                    best_node=current_node;
            }
            current_node=current_node->next;
        }
        bool found_a_potential_free_block = best_node != nullptr;
        if(!found_a_potential_free_block) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- search failure:: no block was found"<< std::endl;
#endif
            return nullptr;
        } else {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- search success:: fit block was found of size "
            << best_node->base.size() << " bytes" << std::endl;
#endif
        }

        auto * resolved_header = split_free_block_to_two_by_payload_size(best_node,
                                                                         size_bytes);

        // remove resolved_header from linked list
        bool is_resolved_block_first = resolved_header->prev==nullptr;
        bool is_resolved_block_last = resolved_header->next==nullptr;
        if(!is_resolved_block_first)
            resolved_header->prev->next = resolved_header->next;
        if(!is_resolved_block_last)
            resolved_header->next->prev = resolved_header->prev;
        if(is_resolved_block_first)
            _free_list_root = resolved_header->next;
        // this is really optional, since this space will become part of the payload
        resolved_header->prev=resolved_header->next=nullptr;
        get_block(ptr_to_int(resolved_header)).toggle_allocated();
        //

        auto address = ptr_to_int(resolved_header) + align_up(size_of_block_base_header());
        _allocations += resolved_header->base.size();
#ifdef DEBUG_ALLOCATOR
        std::cout << "- fulfilled:: block of size " << resolved_header->base.size();
        std::cout << " bytes (aligned up)"<< std::endl;
        std::cout << "              address is " << address << std::endl;
        print(true);
#endif
        void * ptr = int_to_ptr(address);
        return ptr;
    }

    bool free(void * pointer) override {
        auto address = this->ptr_to_int(pointer);

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "FREE:: dynamic allocator" << std::endl
                  << "- address @ " << address << std::endl;
#endif
        if(!is_aligned(address)) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error: address is misaligned to " << this->alignment << " bytes" << std::endl;
#endif
            return false;
        }

        // get the base header
        uptr header_address = address - align_up(size_of_block_base_header());
        const auto block = get_block(header_address);
        const bool sanity_check = block.sanity_test();
        if(!sanity_check) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- failed sanity check, this is probably not a block address" << std::endl;
#endif
            return false;
        }
#ifdef DEBUG_ALLOCATOR
        std::cout << "- found block: size " << block.size() << " @" << block.aligned_from << std::endl;
        std::cout << "               allocation stat is ";
        std::cout << (block.is_allocated() ? "allocated" : "free") << std::endl;
#endif
        if(!block.is_allocated()) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error block is marked as Free !!!" << std::endl;
#endif
            return false;
        }
        // mark it as free, while this might seem redundant as we create a new free block later,
        // BUT, in case of coalesce from left to this block, this can help the allocator to deny
        // false multiple de-allocation of user fault behaviour
        block.toggle_allocated();
        _allocations -= block.size();

        bool is_first_block = align_up(this->ptr_to_int(_ptr)) == block.aligned_from;
        bool is_last_block = align_down(this->ptr_to_int(_ptr)+_size) == block.aligned_to;
        // coalesce left and right of block
        header_t * left_hint_node=nullptr, * right_hint_node=nullptr;
        uptr left_most_address = block.aligned_from;
        uptr right_most_address = block.aligned_to;
        if(!is_first_block) { // let's try to coalesce left
            // first find left block
            uptr left_block_footer_address = block.aligned_from - align_up(size_of_block_footer());
            auto * left_footer = this->template int_to<footer_t *>(left_block_footer_address);
            uptr left_block_address = block.aligned_from - left_footer->size();
            auto left_block = get_block(left_block_address);
            auto is_allocated = left_block.is_allocated();
#ifdef DEBUG_ALLOCATOR
            std::cout << "- found left block: size " << left_block.size();
            std::cout << ", allocation status is " << (is_allocated?"Allocated, skipping":"Free");
            std::cout << std::endl;
#endif
            if(!is_allocated) { // left block is free, let's remove from list
                bool is_resolved_block_first = left_block.header()->prev==nullptr;
                bool is_resolved_block_last = left_block.header()->next==nullptr;
                if(!is_resolved_block_first)
                    left_block.header()->prev->next = left_block.header()->next;
                if(!is_resolved_block_last)
                    left_block.header()->next->prev = left_block.header()->prev;
                if(is_resolved_block_first)
                    _free_list_root = left_block.header()->next;
                left_most_address=left_block.aligned_from;
                left_hint_node = left_block.header()->prev;
            }
        }

        if(!is_last_block) { // let's try to coalesce right
            // first find right block
            uptr right_block_header_address = block.aligned_to;
            auto right_block = get_block(right_block_header_address);
            auto is_allocated = right_block.is_allocated();
#ifdef DEBUG_ALLOCATOR
            std::cout << "- found right block: size " << right_block.size()
            << ", allocation status is " << (is_allocated?"Allocated, skipping":"Free")
            << std::endl;
#endif
            if(!is_allocated) { // right block is free, let's remove from list
                bool is_resolved_block_first = right_block.header()->prev==nullptr;
                bool is_resolved_block_last = right_block.header()->next==nullptr;
                if(!is_resolved_block_first)
                    right_block.header()->prev->next = right_block.header()->next;
                if(!is_resolved_block_last)
                    right_block.header()->next->prev = right_block.header()->prev;
                if(is_resolved_block_first)
                    _free_list_root = right_block.header()->next;
                right_most_address=right_block.aligned_to;
                right_hint_node = right_block.header()->next;
            }
        }

        // free list might be null
        bool is_free_list_empty = _free_list_root==nullptr;

        // now let's create the bigger block
        auto new_block = create_free_block(left_most_address, right_most_address);

#ifdef DEBUG_ALLOCATOR
        std::cout << "- new free block: size " << new_block.size() << ", spans addresses ["
        << new_block.aligned_from << "-" << new_block.aligned_to << "]" << std::endl;
#endif
        if(is_free_list_empty) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- free list was empty, assigned the block" << std::endl;
#endif
            _free_list_root = new_block.header();
        }
        // if coalesce happened, then we can use the locations to insert
        // into the sorted list in O(1) instead of searching
        else if(left_hint_node) { // insert to the right of hint
            new_block.header()->next = left_hint_node->next;
            new_block.header()->prev = left_hint_node;
            if(left_hint_node->next)
                left_hint_node->next->prev = new_block.header();
            left_hint_node->next = new_block.header();
#ifdef DEBUG_ALLOCATOR
            std::cout << "- used left hint node" << std::endl;
#endif
        }
        else if(right_hint_node) { // insert to the left of hint
            new_block.header()->next = right_hint_node;
            new_block.header()->prev = right_hint_node->prev;
            if(right_hint_node->prev)
                right_hint_node->prev->next = new_block.header();
            right_hint_node->prev = new_block.header();
            // we insert to the left of hint, and if hint was the root,
            // we need to replace
            if(_free_list_root==right_hint_node)
                _free_list_root=new_block.header();
#ifdef DEBUG_ALLOCATOR
            std::cout << "- used right hint node" << std::endl;
#endif
        }
        else { // search the entire free list in ascending address order
#ifdef DEBUG_ALLOCATOR
            std::cout << "- searching the entire free list, ascending order" << std::endl;
#endif
            auto * current_node = _free_list_root;
            auto * current_node_before = current_node;
            while (current_node && (ptr_to_int(current_node) < new_block.aligned_from)) {
                current_node_before=current_node;
                current_node=current_node->next;
            }
            if(current_node==nullptr) { // we reached the end, make it last node
#ifdef DEBUG_ALLOCATOR
                std::cout << "- block was inserted last" << std::endl;
#endif
                current_node_before->next=new_block.header();
                new_block.header()->prev = current_node_before;
                new_block.header()->next = nullptr;
            } else { // insert to the left
                new_block.header()->prev = current_node->prev;
                new_block.header()->next = current_node;
                if(current_node->prev)
                    current_node->prev->next=new_block.header();
                else {
#ifdef DEBUG_ALLOCATOR
                    std::cout << "- block was inserted first" << std::endl;
#endif
                    _free_list_root = new_block.header();
                }

                current_node->prev=new_block.header();
            }

        }

        print(true);
        return true;
    }

    void print(bool embed) const override {
#ifdef DEBUG_ALLOCATOR
        if(!embed)
            std::cout << std::endl << "PRINT:: dynamic allocator " << std::endl;
        if(!_free_list_root) {
            std::cout << "- free list is empty" << std::endl;
            return;
        }
        auto * current_node = _free_list_root;
        std::cout << "- free blocks [";
        while (current_node) {
            std::cout << current_node->base.size() << (current_node->next?"->":"]");
            current_node=current_node->next;
        }
        uptr diff_space = end_aligned_address()-start_aligned_address();
        std::cout << std::endl << "- available size [" << available_size() << "/" << diff_space << "]" << std::endl;
        std::cout << std::endl;
#endif
    }

    bool is_equal(const memory_resource<> &other) const noexcept override {
        bool equals = this->type_id() == other.type_id();
        if(!equals) return false;
        const auto * casted_other = static_cast<const dynamic_memory<> *>(&other);
        equals = this->_ptr==casted_other->_ptr;
        return equals;
    }

};
