#pragma once

#define DEBUG_ALLOCATOR

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
 *         the highest alignment requirement that you wish to store in the memory dynamic_allocator.
 *         alignment of atomic types usually equals their size.
 *         alignment of struct types equals the maximal alignment among it's member types.
 *         if you have std lib, you can infer these, otherwise, just plug them if you know
 *
 * @author Tomer Riko Shalev
 */
template<typename uintptr_type=unsigned long,
        uintptr_type alignment=sizeof(uintptr_type)>
                class pool_allocator {
        private:
            using uint = unsigned int;
            using uptr = uintptr_type;

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

            uint free_blocks_count() {
                return _free_blocks_count;
            }

            /**
             *
             * @param ptr start of memory
             * @param size_bytes the memory size in bytes
             * @param block_size the block size
             * @param guard_against_double_free if {True}, user will not be able to accidentally
             *          free an already free block at the cost of having free operation at O(free-list-size).
             *          If {False}, free will take O(1) operations like allocations.
             */
            pool_allocator(void * ptr, uint size_bytes, uint block_size,
                           bool guard_against_double_free=false) :
            _ptr(ptr), _size(size_bytes), _block_size(0),
            _guard_against_double_free(guard_against_double_free) {
#ifdef DEBUG_ALLOCATOR
                std::cout << std::endl << "mem pool_allocator hello"<< std::endl;
                std::cout << "* correct block size due to headers and alignment is "
                << correct_block_size(block_size) << " bytes" <<std::endl;
                std::cout << "* requested alignment is " << alignment << " bytes" << std::endl;
#endif
                _ptr = ptr;
                const bool is_memory_valid = correct_block_size(block_size) <= size_bytes;
                if(is_memory_valid) {
                    reset(block_size);
#ifdef DEBUG_ALLOCATOR
                    std::cout << "* first block @ " << ptr_to_int(_free_list_root) << std::endl;
#endif
                } else {
#ifdef DEBUG_ALLOCATOR
                    std::cout << "* memory does not satisfy minimal size requirements !!!"
                    << std::endl;
#endif
                }

                print_free_list();
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

            void * allocate(uptr size_bytes_dont_matter=0) {
                if(_free_list_root==nullptr) {
#ifdef DEBUG_ALLOCATOR
                    std::cout << std::endl << "ALLOCATE:: no free blocks are available"
                              << std::endl;
#endif
                    return nullptr;
                }
                auto * current_node = _free_list_root;
                _free_list_root = _free_list_root->next;
                _free_blocks_count-=1;

#ifdef DEBUG_ALLOCATOR
                std::cout << std::endl << "ALLOCATE:: handed a free block @"
                          << ptr_to_int(current_node) << std::endl;
#endif

                print_free_list();
                return current_node;
            }

            bool free(void * pointer) {
                auto address = ptr_to_int(pointer);

#ifdef DEBUG_ALLOCATOR
                std::cout << std::endl << "FREE:: address @ " << address <<std::endl;
                uptr min_range = align_up(ptr_to_int(_ptr));
                uptr max_range = align_down(ptr_to_int(_ptr) + _size);
                bool is_in_range = address >= min_range && address < max_range;
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
                print_free_list();
                return true;
            }

            void print_free_list() const {
#ifdef DEBUG_ALLOCATOR
                std::cout << std::endl << "PRINT:: free list " << std::endl;
                std::cout << "- free list is [" << _free_blocks_count << "/" << _blocks_count << "]" << std::endl;
                std::cout << std::endl;
#endif
            }

        };
