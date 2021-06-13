#pragma once

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
 *         the highest alignment requirement that you wish to store in the memory dynamic_allocator.
 *         alignment of atomic types usually equals their size.
 *         alignment of struct types equals the maximal alignment among it's member types.
 *         if you have std lib, you can infer these, otherwise, just plug them if you know
 *
 * @author Tomer Riko Shalev
 */
template<typename uintptr_type=unsigned long, uintptr_type alignment=sizeof(uintptr_type)>
class linear_allocator {
private:
    using uint = unsigned int;
    using uptr = uintptr_type;

    void * _ptr=nullptr;
    void * _current_ptr=nullptr;
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
    T int_to(uptr integer) { return reinterpret_cast<T>(integer); }

public:

    /**
     *
     * @param ptr start of memory
     * @param size_bytes the memory size in bytes
     * @param block_size the block size
     * @param guard_against_double_free if {True}, user will not be able to accidentally
     *          free an already free block at the cost of having free operation at O(free-list-size).
     *          If {False}, free will take O(1) operations like allocations.
     */
    linear_allocator(void * ptr, uint size_bytes) :
                            _ptr(ptr), _size(size_bytes) {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "HELLO:: linear allocator"<< std::endl;
        std::cout << "* requested alignment is " << alignment << " bytes" << std::endl;
#endif
        _ptr = ptr;
        reset();
    }

    void reset() {
        _current_ptr = int_to<void *>(align_up(ptr_to_int(_ptr)));

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "RESET:: linear allocator" << std::endl
                  << "- reset memory to start @ " << ptr_to_int(_current_ptr)
                  << " (aligned up)" << std::endl;
#endif
    }

    uptr available_size() const {
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

    void * allocate(uptr size_bytes=0) {
        size_bytes = align_up(size_bytes);
        const uptr available_space = available_size();
        const bool has_available_size = size_bytes <= available_space;
        const bool has_requested_size_zero = size_bytes==0;

#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "ALLOCATE:: linear allocator" << std::endl
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
        _current_ptr = int_to<void *>(ptr_to_int(pointer) + size_bytes);

        return pointer;
    }

    bool free(void * pointer) {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "FREE:: linear allocator "
        << std::endl << "- linear allocator does not free space, use reset() instead "
        << std::endl << "- available size is " << available_size()
        << std::endl;
#endif
        return true;
    }

    void print() const {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "PRINT:: linear allocator "
                  << std::endl << "- available size is " << available_size()
                  << std::endl;
#endif
    }

};
