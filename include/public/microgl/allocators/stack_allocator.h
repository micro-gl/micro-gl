#pragma once

#ifdef DEBUG_ALLOCATOR
#include <iostream>
#endif

/**
 * stack block allocator
 *
 * free is O(1)
 * allocations are O(1)
 *
 * notes:
 * - allocations and de-allocations are SAFE. de-allocations will fail if trying
 *   to free NOT the latest allocated block.
 * - each allocation requests a block + a small footer, that holds the size of the block
 *   including the size of aligned footer.
 * - upon free operations, the footer is read and compared to the free address in order
 *   to validate the LIFO property of the stack.
 * - at all times, we keep track at the address after the end of the stack
 * - minimal block size is 4 bytes for 32 bit pointer types and 8 bytes for 64 bits pointers.
 * - blocks print is user_space = block_size - size_of_aligned_footer
 *
 * @tparam uintptr_type unsigned integer type that can hold a pointer
 * @tparam alignment alignment requirement, must be valid power of 2, that can satisfy
 *         the highest alignment requirement that you wish to store in the memory dynamic_allocator.
 *         alignment of atomic types usually equals their size.
 *         alignment of struct types equals the maximal alignment among it's member types.
 *         if you have std lib, you can infer these, otherwise, just plug them if you know
 *
 * @author Tomer Riko Shalev
 */
template<typename uintptr_type=unsigned long, uintptr_type alignment=sizeof(uintptr_type)>
class stack_allocator {
private:
    using uint = unsigned int;
    using uptr = uintptr_type;

    struct footer_t {
        uptr size=0;
    };

    void * _ptr=nullptr;
    uptr _current_head = 0;
    uint _size=0;

    static
    inline uptr align_up(const uptr address)
    {
        constexpr uptr align_m_1 = alignment - 1;
        constexpr uptr b = ~align_m_1;
        uptr a = (address+align_m_1);
        uptr c = a & b;
        return c;
    }

    static inline
    uptr is_aligned(const uptr address) { return align_down(address)==address; }

    static inline uptr align_down(const uptr address)
    {
        constexpr uptr a = ~(alignment - 1);
        return (address & a);
    }

    static uptr ptr_to_int(void * pointer) { return reinterpret_cast<uptr>(pointer); }
    static void * int_to_ptr(uptr integer) { return reinterpret_cast<void *>(integer); }

    template<typename T>
    T int_to(uptr integer) const { return reinterpret_cast<T>(integer); }

    uptr aligned_size_of_footer() const {
        return align_up(sizeof (footer_t));
    }

public:
    uptr start_aligned_address() const {
        return align_up(ptr_to_int(_ptr));
    }

    uptr end_aligned_address() const {
        return align_down(ptr_to_int(_ptr) + _size);
    }

    /**
     * ctor
     *
     * @param ptr start of memory
     * @param size_bytes the memory size in bytes
     */
    stack_allocator(void * ptr, uint size_bytes) :
        _ptr(ptr), _size(size_bytes) {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "HELLO:: stack allocator hello"<< std::endl;
        std::cout << "* minimal block size due to headers and alignment is "
                  << aligned_size_of_footer() << " bytes" << std::endl;
        std::cout << "* requested alignment is " << alignment << " bytes" << std::endl;
#endif
        const bool is_memory_valid = aligned_size_of_footer() <= size_bytes;
        if(!is_memory_valid) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "* memory does not satisfy minimal size requirements !!!"
            << std::endl;
#endif
        } else
            _current_head = align_up(ptr_to_int(_ptr));
    }

    uptr available_size() const {
        const uptr min = align_up(_current_head);
        const uptr max = end_aligned_address();
        const uptr delta = max - min;
        return delta;
    }

    void * allocate(uptr size_bytes=0) {
        size_bytes = align_up(size_bytes);
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "ALLOCATE:: stack allocator"
                  << std::endl << "- requested " << size_bytes << "bytes (aligned up)"
                  << std::endl;
#endif
        if(size_bytes==0)
            return nullptr;

        size_bytes = align_up(size_bytes) + aligned_size_of_footer();

        bool has_space = size_bytes<=available_size();
        if(!has_space) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- no free space available " << available_size() << std::endl;
#endif
            return nullptr;
        }
        uptr block_start = _current_head;

        _current_head += size_bytes;
        footer_t * footer = int_to<footer_t *>(_current_head - aligned_size_of_footer());
        footer->size=size_bytes;
#ifdef DEBUG_ALLOCATOR
        std::cout << "- handed a free block @"
                  << block_start << std::endl;
#endif

#ifdef DEBUG_ALLOCATOR
        print(true);
#endif
        return int_to<void *>(block_start);
    }

    bool free(void * pointer) {
        auto address = ptr_to_int(pointer);

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "FREE:: stack allocator " << std::endl
        << "- free a block address @ " << address << std::endl;
#endif
        const bool is_empty = _current_head==start_aligned_address();
        if(is_empty) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error: nothing was allocated, nothing to free"
                      << std::endl;
#endif
            return false;
        }

        footer_t * footer = int_to<footer_t *>(_current_head - aligned_size_of_footer());
        bool is_lifo = address==(_current_head-footer->size);

        if(!is_lifo) {
#ifdef DEBUG_ALLOCATOR
            std::cout << "- error: proposed free block is not the latest allocated, "
                         "and thus violating the LIFO property !!!" << std::endl;
#endif
            return false;
        }

        _current_head -= footer->size;
#ifdef DEBUG_ALLOCATOR
        print(true);
#endif
        return true;
    }

    void print(bool embed=false) const {
#ifdef DEBUG_ALLOCATOR
        if(!embed)
            std::cout << std::endl << "PRINT:: stack allocator " << std::endl;
        std::cout << "- blocks (LIFO order) [";
        uptr root = align_up(ptr_to_int(_ptr));
        uptr head = _current_head;
        bool is_last = true;
        while(head>root) {
            footer_t * footer = int_to<footer_t *>(head - aligned_size_of_footer());
            head-=footer->size;
            bool is_first = !(head>root);
            std::cout << (!is_last?"<-":"") << footer->size - aligned_size_of_footer() << (is_first ? "(ROOT)" : "");
            if(is_last) is_last=false;
        }
        std::cout << "]" << std::endl;
#endif
    }

};
