#include <iostream>
//#include <vector>
//#include <new>
#include <cstdint>
class allocator {
private:
    using uint = unsigned int;
    using uptr = unsigned long;
    using byte_ptr = unsigned char *;
    void * _ptr;
//    uptr _ptr;
    uint _size;
    uint _alignment;

    static inline uptr align_up(const uptr address, uint alignment)
    {
        uptr align_m_1 = alignment - 1;
        uptr a = (address+align_m_1);
        uptr b = ~align_m_1;
        uptr c = a & b;
        return c;
    }

    static
    inline uptr is_aligned(const uptr address, uint alignment)
    {
        return align_down(address, alignment)==address;
    }

    inline uptr is_aligned(const uptr address)
    {
        return is_aligned(address, _alignment);
    }

    inline uptr align_up(const uptr address) const
    {
        return align_up(address, _alignment);
    }

    static inline uptr align_down(const uptr address, uint alignment)
    {
        uptr align_m_1 = alignment - 1;
        return (address & (~align_m_1));
    }

    inline uptr align_down(const uptr address) const
    {
        return align_down(address, _alignment);
    }

    static uptr ptr_to_int(void * pointer) {
        return reinterpret_cast<uptr>(pointer);
    }

    static void * int_to_ptr(uptr integer) {
        return reinterpret_cast<void *>(integer);
    }

    template<typename T>
    T int_to(uptr integer) {
        return reinterpret_cast<T>(integer);
    }

    byte_ptr ptr_to_byte_ptr(void * pointer) {
        return reinterpret_cast<byte_ptr>(pointer);
    }

public:

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

    header_t * int_to_header_ptr(uptr integer) {
        return reinterpret_cast<header_t *>(integer);
    }

    struct block_t {
        uptr aligned_from=0, aligned_to=0;
        header_t * header() const {
            return reinterpret_cast<header_t *>(aligned_from);
        }
        footer_t * footer() const {
            return reinterpret_cast<footer_t *>(aligned_to -
                            align_up(sizeof (footer_t), 8));
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
    };

    block_t create_free_block(uptr from, uptr to) const {
        block_t result;
        result.aligned_from = align_up(from);
        result.aligned_to = align_down(to);
        result.set_size_and_status(result.aligned_to-result.aligned_from, false);
        result.header()->prev=nullptr;
        result.header()->next=nullptr;
        return result;
    }

    block_t get_block(uptr from) const {
        block_t result;
        result.aligned_from = align_up(from, _alignment);
        result.aligned_to = result.aligned_from+result.size();
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
        uptr required_free_block_size = minimal_size_of_any_block() + _alignment;
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

            std::cout << "- split:: from size [" << old_size << "] bytes (aligned up) "
                    << "into two blocks of sizes [" << block_1.size() << ":" << block_2.size()
                     << "]" << std::endl;

            return block_1.header();
        }
        return block;
    }

    header_t *_free_list_root = nullptr;
public:
    allocator(void * ptr, unsigned int size_bytes, unsigned int alignment) :
                            _alignment(alignment), _ptr(ptr), _size(size_bytes) {
        std::cout << std::endl << "mem allocator hello"<< std::endl;
        std::cout << "* minimal block size due to headers, footers and alignment is "
                  << minimal_size_of_any_block() << " bytes" <<std::endl;
        std::cout << "* requested alignment is " << _alignment << " bytes" << std::endl;

        _ptr = ptr;//int_to_ptr(from);
        auto block = create_free_block(ptr_to_int(ptr), ptr_to_int(ptr) + size_bytes);
        bool is_memory_valid = block.size() >= minimal_size_of_any_block();
        std::cout << "* principal memory block after alignment is " << block.size() << " bytes"
                  << std::endl;
        if(is_memory_valid) {
            _free_list_root = block.header();
        } else {
            std::cout << "* memory does not satisfy minimal size requirements !!!"
                      << std::endl;
        }

        print_free_list();
    }

    void * allocate(uptr size_bytes) {
        size_bytes = align_up(size_bytes);
        std::cout << std::endl << "ALLOCATE:: requested " << size_bytes
                  << " bytes (aligned up)"<< std::endl;
        auto * current_node = _free_list_root;
        header_t * best_node= nullptr;
        std::cout << "- search :: searching for max(" << size_bytes +
                    aligned_base_header_and_footer()
                  << ", " << minimal_size_of_any_block() << ") bytes block due to align, "
                                                            "base header and footer"
                  << std::endl;
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
            std::cout << "- search failure:: no block was found"<< std::endl;
            return nullptr;
        } else {
            std::cout << "- search success:: fit block was found of size "
                      << best_node->base.size() << " bytes" << std::endl;
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
//        resolved_header->base.toggle_allocated();
        get_block(ptr_to_int(resolved_header)).toggle_allocated();
        //

        auto address = ptr_to_int(resolved_header) + align_up(size_of_block_base_header());

        std::cout << "- fulfilled:: block of size " << resolved_header->base.size()
                  << " bytes (aligned up)"<< std::endl;
        std::cout << "              address is " << address << std::endl;
        print_free_list();

        void * ptr = int_to_ptr(address);
        return ptr;
    }

    bool free(void * pointer) {
        auto address = ptr_to_int(pointer);

        std::cout << std::endl << "FREE:: address @ " << address <<std::endl;

        if(!is_aligned(address)) {
            std::cout << "- error: address is misaligned to "
                      << _alignment << " bytes" << std::endl;
            return false;
        }

        // get the base header
        uptr header_address = address - align_up(size_of_block_base_header());
        auto block = get_block(header_address);
        std::cout << "- found block: size " << block.size() << " @" << block.aligned_from <<std::endl;
        std::cout << "               allocation stat is "
                  << (block.is_allocated() ? "allocated" : "free") << std::endl;
        if(!block.is_allocated()) {
            std::cout << "- error block is marked as Free !!!" << std::endl;
            return false;
        }

        bool is_first_block = align_up(ptr_to_int(_ptr)) == block.aligned_from;
        bool is_last_block = align_down(ptr_to_int(_ptr)+_size) == block.aligned_to;
        // coalesce left and right of block
        header_t * left_hint_node=nullptr, * right_hint_node=nullptr;
        uptr left_most_address = block.aligned_from;
        uptr right_most_address = block.aligned_to;
        if(!is_first_block) { // let's try to coalesce left
            // first find left block
            uptr left_block_footer_address = block.aligned_from - align_up(size_of_block_footer());
            auto * left_footer = int_to<footer_t *>(left_block_footer_address);
            uptr left_block_address = block.aligned_from - left_footer->size();
            auto left_block = get_block(left_block_address);
            auto is_allocated = left_block.is_allocated();
            std::cout << "- found left block: size " << left_block.size()
            << ", allocation status is " << (is_allocated?"Allocated, skipping":"Free")
            << std::endl;

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
            std::cout << "- found right block: size " << right_block.size()
            << ", allocation status is " << (is_allocated?"Allocated, skipping":"Free")
            << std::endl;

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

        std::cout << "- new free block: size " << new_block.size() << ", spans addresses ["
                  << new_block.aligned_from << "-" << new_block.aligned_to << "]" << std::endl;

        if(is_free_list_empty) {
            std::cout << "- free list was empty, assigned the block" << std::endl;
            _free_list_root = new_block.header();
        }
        else if(left_hint_node) { // insert link
            new_block.header()->next = left_hint_node->next;
            new_block.header()->prev = left_hint_node;
            if(left_hint_node->next)
                left_hint_node->next->prev = new_block.header();
            left_hint_node->next = new_block.header();
            std::cout << "- used left hint node" << std::endl;
        }
        else if(right_hint_node) {
            new_block.header()->next = right_hint_node;
            new_block.header()->prev = right_hint_node->prev;
            if(right_hint_node->prev)
                right_hint_node->prev->next = new_block.header();
            right_hint_node->prev = new_block.header();
            std::cout << "- used right hint node" << std::endl;
        }
        else { // search the entire list in order ascending
            std::cout << "- searching the entire free list, ascending order" << std::endl;
            auto * current_node = _free_list_root;
            auto * current_node_before = current_node;
            while (current_node && (ptr_to_int(current_node) < new_block.aligned_from)) {
                current_node_before=current_node;
                current_node=current_node->next;
            }
            if(current_node==nullptr) { // we reached the end, make it last node
                std::cout << "- block was inserted last" << std::endl;
                current_node_before->next=new_block.header();
                new_block.header()->prev = current_node_before;
                new_block.header()->next = nullptr;
            } else { // insert to the left
                new_block.header()->prev = current_node->prev;
                new_block.header()->next = current_node;
                if(current_node->prev)
                    current_node->prev->next=new_block.header();
                else {
                    std::cout << "- block was inserted first" << std::endl;
                    _free_list_root = new_block.header();
                }

                current_node->prev=new_block.header();
            }

        }

        print_free_list();
        return true;
    }

    void print_free_list() const {
        std::cout << std::endl << "PRINT:: free list " << std::endl;
        if(!_free_list_root) {
            std::cout << "- free list is empty" << std::endl;
            return;
        }
        auto * current_node = _free_list_root;
        int ix=0;
        std::cout << "- free blocks [";
        while (current_node) {
            std::cout << current_node->base.size() << (current_node->next?"->":"]");
            current_node=current_node->next;
        }
        std::cout << std::endl;
    }

};

struct aaa {
//    char size=0;
//    char prev=0;
char bbb[3];
};

int main() {
    using byte= unsigned char;
    const int size = 1024+16;
    byte memory[size];

    allocator alloc{memory, size, 8};

    void * a1 = alloc.allocate(200);
    void * a2 = alloc.allocate(200);
    void * a3 = alloc.allocate(200);
    void * a4 = alloc.allocate(200);
//    alloc.free(a1);
//    alloc.free(a2);
//    alloc.free(a3);
    alloc.free(a4);

//    alloc.free(a3);


//    alloc.print_free_list();

    std::cout << sizeof (aaa) << std::endl;
    std::cout << alignof(aaa) << std::endl;
//    std::cout << offsetof(aaa, prev) << std::endl;
}

