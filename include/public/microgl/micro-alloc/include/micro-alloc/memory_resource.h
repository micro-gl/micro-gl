#pragma once

/**
 * memory resource is a runtime polymorphic interface for handling memory allocations
 * @tparam uintptr_type unsigned integer type that is the size of a pointer
 */
template<typename uintptr_type=unsigned long>
class memory_resource {
private:
    const char _type_id;
protected:
    bool _is_valid;
public:
    using uint = unsigned int;
    using uptr = uintptr_type;

    uptr alignment;

    inline uptr align_up(const uptr address) const
    {
        uptr align_m_1 = alignment - 1;
        uptr b = ~align_m_1;
        uptr a = (address+align_m_1);
        uptr c = a & b;
        return c;
    }

    inline
    uptr is_aligned(const uptr address) const { return align_down(address)==address; }

    inline uptr align_down(const uptr address) const
    {
        uptr a = ~(alignment - 1);
        return (address & a);
    }

    static uptr ptr_to_int(const void * pointer) { return reinterpret_cast<uptr>(pointer); }
    static void * int_to_ptr(uptr integer) { return reinterpret_cast<void *>(integer); }

    template<typename T>
    static T int_to(uptr integer) { return reinterpret_cast<T>(integer); }

    char type_id() const {
        return _type_id;
    }

public:
    /**
     * ctor
     * @param type_id unique type id of a memory resource type implementation
     * @param alignment alignment requirement, needs to be a power of 2 that is divisible by sizeof(uintptr_type)
     */
    explicit memory_resource(char type_id=-1, uptr alignment= sizeof(uintptr_type)) :
                _type_id{type_id}, alignment{alignment}, _is_valid(true) {}
    virtual ~memory_resource() = default;

    /**
     * is this memory resource a valid one ?
     */
    bool is_valid() const { return _is_valid; };

    /**
     * allocate raw memory
     * @param size_bytes number of bytes to allocate
     * @return a pointer on success or {nullptr}
     */
    virtual void * malloc(uptr size_bytes) = 0;

    /**
     * free an allocated pointer
     * @param pointer pointer to free
     * @return {true/false} on success/failure
     */
    virtual bool free(void * pointer) = 0;

    /**
     * get the available size in bytes in this memory resource
     */
    virtual uptr available_size() const { return 0; };

    /**
     * print something
     */
    virtual void print(bool embed) const {};

    /**
     * Two memory_resources compare equal if and only if memory allocated
     * from one memory_resource can be deallocated from the other and vice versa.
     */
    virtual bool is_equal(const memory_resource& other) const noexcept {
        return false;
    }

};

template<typename uintptr_type>
bool operator==( const memory_resource<uintptr_type>& a,
                 const memory_resource<uintptr_type>& b ) noexcept {
    return  &a == &b || a.is_equal(b);
}