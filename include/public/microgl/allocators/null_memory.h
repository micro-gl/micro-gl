#pragma once

#include "memory_resource.h"

//#define DEBUG_ALLOCATOR

#ifdef DEBUG_ALLOCATOR
#include <iostream>
#endif

/**
 * null memory, doesn't do anything
 *
 * @author Tomer Riko Shalev
 */
class null_memory : public memory_resource<unsigned long> {
private:
    using base = memory_resource<unsigned long>;

public:

    /**
     * ctor
     *
     * @param alignment alignment requirement
     */
    explicit std_memory(uptr alignment=sizeof (unsigned long)) :
            base{0, alignment} {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "HELLO:: null memory resource"<< std::endl;
        std::cout << "* requested alignment is " << alignment << " bytes" << std::endl;
#endif
    }

    uptr available_size() const override {
        return uptr(0);
    }

    void * malloc(uptr size_bytes) override {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "MALLOC:: null memory" << std::endl
                  << "- request a block of size " << size_bytes
                  << std::endl;
#endif
        return nullptr;
    }

    bool free(void * pointer) override {
#ifdef DEBUG_ALLOCATOR
        std::cout << std::endl << "FREE:: null memory " << std::endl;
#endif
        return false;
    }

    void print(bool embed) const override {
    }

    bool is_equal(const memory_resource<> &other) const noexcept override {
        bool equals = this->type_id() == other.type_id();
        return equals;
    }

};
